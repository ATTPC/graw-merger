//
//  Merger.cpp
//  get-manip
//
//  Created by Joshua Bradt on 9/19/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Merger.h"

using namespace getmanip;

Merger::Merger()
{}

int Merger::AddFramesFromFileToIndex(const boost::filesystem::path& fpath)
{
    // Check if the file has already been read
    if (files.find(fpath.filename().string()) != files.end()) {
        throw getevt::Exceptions::File_Already_Read(fpath.filename().string());
    }
    
    // Open the file
    std::shared_ptr<getevt::GRAWFile> file {new getevt::GRAWFile(fpath, std::ios::in)};
    
    int nFramesRead {0};
    
    // Index the frames in the file
    while (!file->eof()) {
        try {
            auto fm = file->ReadFrameMetadata();
            MergingMapEntry me {};
            me.filePtr = file;
            me.filePos = fm.filePos;
            
            mmap.emplace(fm.evtId, me);
            nFramesRead++;
        }
        catch (getevt::Exceptions::Frame_Read_Error& frErr) {
            std::cerr << frErr.what() << std::endl;
            continue;
        }
        catch (std::exception& e) {
            std::cout << "Exception: " << e.what() << std::endl;
            throw;
        }
    }
    
    // Put the file in the list of files we've already read. This also copies
    // the shared pointer, which keeps the file around after this function
    // returns.
    
    files.emplace(fpath.filename().string(),file);
    return nFramesRead;
}

void Merger::MergeByEvtId(const std::string &outfilename, getevt::PadLookupTable* lt,
                          getevt::LookupTable<getevt::sample_t>& pedsTable, bool suppZeros,
                          getevt::sample_t threshold)
{
    getevt::EventFile outfile;
    outfile.OpenFileForWrite(outfilename);
    
    std::cout << "Beginning merge of " << mmap.size() << " frames." << std::endl;
    
    // Find first event and number of events
    uint32_t firstEvtId = mmap.lower_bound(0)->first;
    uint32_t numEvts = mmap.rbegin()->first - firstEvtId;
    
    int nWorkers = std::thread::hardware_concurrency();
    std::vector<std::thread> workers;
    for (int i = 0; i < nWorkers; i++) {
        workers.push_back(std::thread {[this]{ return TaskWorker(); }});
    }
    
    std::thread writer {[this, &outfile]{ return ResultWriter(outfile); }};
    
    // Create events and append to file
    for (uint32_t currentEvtId = firstEvtId ; mmap.size() > 0; currentEvtId++) {
        // Find frames for this event. equal_range() returns a pair containing
        // iterators to the first and last frame with this event ID. If no frames
        // are found, they'll both point to the next highest event ID.
        auto currentRange = mmap.equal_range(currentEvtId);
        if (currentRange.first->first > currentEvtId) continue; // no frames found
        
        // Extract frames from files
        std::queue<getevt::GRAWFrame> frames;
        for (auto frameRefPtr = currentRange.first; frameRefPtr != currentRange.second; frameRefPtr++) {
            auto& fref = frameRefPtr->second;
            fref.filePtr->SetPosition(fref.filePos);
            std::vector<uint8_t> rawFrame = fref.filePtr->ReadRawFrame();
            frames.push(getevt::GRAWFrame {rawFrame});
        }
        
        EventProcessingTask task {std::move(frames), lt, pedsTable, suppZeros, threshold};
        
        std::packaged_task<getevt::Event()> pt {std::move(task)};
        
        resq.put(pt.get_future());
        
        tq.put(std::move(pt));

        mmap.erase(currentEvtId);
        
        ShowProgress(currentEvtId - firstEvtId, numEvts);
    }
    
    // Now we're done reading events, so cause the threads to finish
    
    tq.finish();
    resq.finish();
    
    for (auto& th : workers) {
        th.join();
    }
    
    writer.join();
}

Merger::EventProcessingTask::EventProcessingTask(std::queue<getevt::GRAWFrame> fr,
                                                 getevt::PadLookupTable* lt,
                                                 getevt::LookupTable<getevt::sample_t> peds,
                                                 bool suppZeros, getevt::sample_t th)
: frames(fr), pads(lt), peds(peds), suppZeros(suppZeros), threshold(th)
{}

getevt::Event Merger::EventProcessingTask::operator()()
{
    getevt::Event res {};
    res.SetLookupTable(pads);
    
    while (!frames.empty()) {
        res.AppendFrame(frames.front());
        frames.pop();
    }
    
    res.SubtractFPN();
    
    if (not peds.Empty()) {
        res.SubtractPedestals(peds);
    }
    
    if (threshold > getevt::Constants::min_sample) {
        res.ApplyThreshold(threshold);
    }
    
    if (suppZeros) {
        res.DropZeros();
    }
    
    return res;
}

template<typename T>
void Merger::SyncQueue<T>::put(const T &task)
{
    std::unique_lock<std::mutex> lock(qmtx);
    cond.wait(lock, [this]{ return q.size() < 20; });
    q.push_back(task);
    cond.notify_all();
}

template<typename T>
void Merger::SyncQueue<T>::put(T &&task)
{
    std::unique_lock<std::mutex> lock(qmtx);
    cond.wait(lock, [this]{ return q.size() < 20;});
    q.push_back(std::move(task));
    cond.notify_all();
}

template<typename T>
void Merger::SyncQueue<T>::get(T &dest)
{
    std::unique_lock<std::mutex> lock(qmtx);
    cond.wait(lock, [this]{ return !q.empty() or finished; });
    if (finished) throw NoMoreTasks();
    dest = std::move(q.front());
    q.pop_front();
    cond.notify_all();
}

template<typename T>
void Merger::SyncQueue<T>::finish()
{
    std::unique_lock<std::mutex> lock {qmtx};
    cond.wait(lock, [this]{ return q.empty(); });
    finished = true;
    cond.notify_all();
}

void Merger::TaskWorker()
{
    while (true) {
        std::packaged_task<PTT> task;
        try {
            tq.get(task);
            task();
        }
        catch (Merger::SyncQueue<std::packaged_task<PTT>>::NoMoreTasks& t) {
            return;
        }
    }
}

void Merger::ResultWriter(getevt::EventFile& of)
{
    while (true) {
        try {
            std::future<getevt::Event> processed_fut;
            resq.get(processed_fut);
            auto processed = processed_fut.get();
            of.WriteEvent(processed);
        }
        catch (Merger::SyncQueue<std::future<getevt::Event>>::NoMoreTasks& t) {
            return;
        }
        catch (std::exception& e) {
            std::cout << "Error in thread: " << e.what() << std::endl;
        }
    }
}

void Merger::ShowProgress(int currEvt, int numEvt)
{
    // Produces a progress bar like this:
    //  50% [==========          ] (50/100)
    // The last part is the current evt / num evts
    
    const int progWidth = 40;
    int pct = floor(float(currEvt) / float(numEvt) * 100);
    std::cout << '\r' << std::string(progWidth,' ') << '\r';
    std::cout << std::setw(3) << pct << "% ";
    std::cout << '[' << std::string(2*pct/10,'=') << std::string(20-2*pct/10,' ') << ']';
    std::cout << " (" << currEvt << "/" << numEvt << ")";
    std::cout.flush();
}