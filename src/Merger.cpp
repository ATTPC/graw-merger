#include "Merger.h"

Merger::Merger()
{
    tq = std::make_shared<taskQueue_type>();
    resq = std::make_shared<futureQueue_type>();
}

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

void Merger::MergeByEvtId(const std::string &outfilename, PadLookupTable* lt)
{
    std::cout << "Beginning merge of " << mmap.size() << " frames." << std::endl;

    // Find first event and number of events
    uint32_t firstEvtId = mmap.lower_bound(0)->first;
    uint32_t numEvts = mmap.rbegin()->first - firstEvtId;

    unsigned nWorkers = std::thread::hardware_concurrency();
    std::vector<TaskWorker> workers;
    for (unsigned i = 0; i < nWorkers; i++) {
        workers.emplace_back(tq, resq);
    }
    for (auto& worker : workers) {
        worker.start();
    }

    HDFWriterWorker writer (outfilename, resq);
    writer.start();

    // Create events and append to file
    for (uint32_t currentEvtId = firstEvtId ; mmap.size() > 0; currentEvtId++) {
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

            try {
                frames.push(GRAWFrame {rawFrame});
            }
            catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }

        EventProcessingTask task {std::move(frames), lt};

        std::packaged_task<Event()> pt {std::move(task)};

        resq->put(pt.get_future());

        tq->put(std::move(pt));

        mmap.erase(currentEvtId);

        ShowProgress(currentEvtId - firstEvtId, numEvts);
    }

    // Now we're done reading events, so cause the threads to finish

    tq->finish();
    resq->finish();

    for (auto& th : workers) {
        th.join();
    }

    writer.join();
}

EventProcessingTask::EventProcessingTask(std::queue<GRAWFrame> fr, PadLookupTable* lt)
: frames(fr), pads(lt)
{}

Event EventProcessingTask::operator()()
{
    Event res {};
    res.SetLookupTable(pads);

    while (!frames.empty()) {
        res.AppendFrame(frames.front());
        frames.pop();
    }

    res.SubtractFPN();

    return res;
}


void TaskWorker::run()
{
    while (true) {
        std::packaged_task<Event(void)> task;
        try {
            inq->get(task);
            task();
        }
        catch (taskQueue_type::NoMoreTasks&) {
            return;
        }
    }
}

void HDFWriterWorker::run()
{
    while (true) {
        try {
            std::future<Event> processed_fut;
            futureq->get(processed_fut);
            auto processed = processed_fut.get();
            hfile.writeEvent(processed);
        }
        catch (futureQueue_type::NoMoreTasks&) {
            return;
        }
        catch (std::exception& e) {
            std::cout << "Error in thread: " << e.what() << std::endl;
        }
    }
}

void Merger::ShowProgress(uint64_t currEvt, uint64_t numEvt)
{
    // Produces a progress bar like this:
    //  50% [==========          ] (50/100)
    // The last part is the current evt / num evts

    const int progWidth = 40;
    unsigned pct = static_cast<unsigned>(std::floor(float(currEvt) / float(numEvt) * 100));
    std::cout << '\r' << std::string(progWidth,' ') << '\r';
    std::cout << std::setw(3) << pct << "% ";
    std::cout << '[' << std::string(2*pct/10,'=') << std::string(20-2*pct/10,' ') << ']';
    std::cout << " (" << currEvt << "/" << numEvt << ")";
    std::cout.flush();
}
