#include "Merger.h"

Merger::Merger(const std::vector<std::string>& filePaths, const std::shared_ptr<PadLookupTable>& lt)
: lookupTable(lt)
{
    tq = std::make_shared<taskQueue_type>();
    resq = std::make_shared<futureQueue_type>();

    for (const auto& path : filePaths) {
        files.emplace_back(path, std::ios::in);
    }

    findex.indexFiles(files);
}

void Merger::MergeByEvtId(const std::string &outfilename, const std::shared_ptr<PadLookupTable>& lt)
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
        std::queue<RawFrame> frames;
        for (auto frameRefPtr = currentRange.first; frameRefPtr != currentRange.second; frameRefPtr++) {
            auto& fref = frameRefPtr->second;
            fref.filePtr->filestream.seekg(fref.filePos);
            RawFrame rf = fref.filePtr->ReadRawFrame();
            frames.push(std::move(rf));
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

Event EventProcessingTask::operator()()
{
    Event res {};
    res.SetLookupTable(pads);

    while (!frames.empty()) {
        GRAWFrame f (frames.front());
        frames.pop();
        res.AppendFrame(f);
    }

    res.SubtractFPN();

    return res;
}


void EventBuilder::run()
{
    while (true) {
        RawFrame raw;
        try {
            rawFrameQueue->get(raw);
        }
        catch (taskQueue_type::NoMoreTasks&) {
            return;
        }

        GRAWFrame frame (raw);
        evtid_t evtid = frame.eventId;


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
