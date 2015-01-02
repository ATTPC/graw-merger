//
//  Merger.cpp
//  get-manip
//
//  Created by Joshua Bradt on 9/19/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "Merger.h"

Merger::Merger()
{}

int Merger::AddFramesFromFileToIndex(const boost::filesystem::path& fpath)
{
    // Check if the file has already been read
    if (files.find(fpath.filename().string()) != files.end()) {
        throw Exceptions::File_Already_Read(fpath.filename().string());
    }
    
    // Open the file
    std::shared_ptr<GRAWFile> file {new GRAWFile(fpath, std::ios::in)};
    
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
        catch (Exceptions::Frame_Read_Error& frErr) {
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

void Merger::MergeByEvtId(const std::string &outfilename, PadLookupTable* lt,
                          LookupTable<sample_t>& pedsTable, bool suppZeros,
                          sample_t threshold)
{
    EventFile outfile;
    outfile.OpenFileForWrite(outfilename);
    
    std::cout << "Beginning merge of " << mmap.size() << " frames." << std::endl;
    
    // Find first event and number of events
    uint32_t firstEvtId = mmap.lower_bound(0)->first;
    uint32_t numEvts = mmap.rbegin()->first - firstEvtId;
    
    // Create events and append to file
    for (uint32_t currentEvtId = firstEvtId ; mmap.size() > 0; )
    {
        std::vector<std::future<Event>> futures {};
        std::vector<std::packaged_task<Event()>> pts {};
        std::vector<std::thread> threads;
        
        for (int qiter = 0; qiter < 8 and mmap.size() > 0; qiter++) {
            // Find frames for this event. equal_range() returns a pair containing
            // iterators to the first and last frame with this event ID. If no frames
            // are found, they'll both point to the next highest event ID.
            auto currentRange = mmap.equal_range(currentEvtId);
            if (currentRange.first->first > currentEvtId) continue; // no frames found
            
            // Extract frames from files
            std::queue<GRAWFrame> frames;
            for (auto frameRefPtr = currentRange.first; frameRefPtr != currentRange.second; frameRefPtr++) {
                auto& fref = frameRefPtr->second;
                fref.filePtr->filestream.seekg(fref.filePos);
                std::vector<uint8_t> rawFrame = fref.filePtr->ReadRawFrame();
                frames.push(GRAWFrame {rawFrame});
            }
            
            EventProcessingTask task {std::move(frames), lt, pedsTable, suppZeros, threshold};
            
            std::packaged_task<Event()> pt {std::move(task)};
            
            futures.push_back(pt.get_future());
            
            threads.push_back(std::thread{std::move(pt)});

            mmap.erase(currentEvtId);
            currentEvtId++;
        }
        
        for (int qiter = 0; qiter < 8; qiter++) {
            threads[qiter].join();
            try {
                auto processed = futures[qiter].get();
                outfile.WriteEvent(processed);
            }
            catch (std::exception& e) {
                std::cout << "Error in thread: " << e.what() << std::endl;
            }
        }
        
        ShowProgress(currentEvtId - firstEvtId, numEvts);
    }
}

Merger::EventProcessingTask::EventProcessingTask(std::queue<GRAWFrame> fr,
                                                 PadLookupTable* lt,
                                                 LookupTable<sample_t> peds,
                                                 bool suppZeros, sample_t th)
: frames(fr), pads(lt), peds(peds), suppZeros(suppZeros), threshold(th)
{}

Event Merger::EventProcessingTask::operator()()
{
    Event res {};
    res.SetLookupTable(pads);
    
    while (!frames.empty()) {
        res.AppendFrame(frames.front());
        frames.pop();
    }
    
    res.SubtractFPN();
    
    if (not peds.Empty()) {
        res.SubtractPedestals(peds);
    }
    
    if (threshold > Constants::min_sample) {
        res.ApplyThreshold(threshold);
    }
    
    if (suppZeros) {
        res.DropZeros();
    }
    
    return res;
}

Event Merger::ProcessEvent(Event evt, const LookupTable<sample_t> pedsTable, const bool suppZeros, const sample_t threshold)
{
    evt.SubtractFPN();
    
    if (not pedsTable.Empty()) {
        evt.SubtractPedestals(pedsTable);
    }
    
    if (threshold > Constants::min_sample) {
        evt.ApplyThreshold(threshold);
    }
    
    if (suppZeros) {
        evt.DropZeros();
    }

    return evt;
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