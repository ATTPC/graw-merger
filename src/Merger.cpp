#include "Merger.h"

Merger::Merger(const std::vector<std::string>& filePaths, const std::shared_ptr<PadLookupTable>& lt)
: lookupTable(lt)
{
    frameQueue = std::make_shared<SyncQueue<RawFrame>>();
    eventQueue = std::make_shared<SyncQueue<Event>>();

    for (const auto& path : filePaths) {
        files.emplace_back(std::make_shared<GRAWFile>(path, std::ios::in));
    }

    findex.indexFiles(files);
}

void Merger::MergeByEvtId(const std::string &outfilename)
{
    std::cout << "Beginning merge" << std::endl;

    EventBuilder builder (frameQueue, eventQueue, lookupTable);
    HDFWriter writer (outfilename, eventQueue);

    builder.start();
    writer.start();

    for (evtid_t currentEvt = 0; files.size() > 0; currentEvt++) {
        std::vector<std::shared_ptr<GRAWFile>> thisEvtFiles = findex.findFilesForEvtId(currentEvt);
        for (auto& file : thisEvtFiles) {
            try {
                for (int i = 0; i < 4*2; i++) {
                    RawFrame fr = file->ReadRawFrame();
                    frameQueue->put(std::move(fr));
                }
            }
            catch (const Exceptions::End_of_File&) {
                // Find the file in *our* list and erase it
                auto doneFileIter = std::find(files.begin(), files.end(), file);
                files.erase(doneFileIter);
            }
        }
    }

    // Now we're done reading frames, so cause the frame queue and threads to finish.
    // The event queue will be finished by the EventBuilder.

    frameQueue->finish();

    builder.join();
    writer.join();
}

bool EventBuilder::eventWasAlreadyWritten(const evtid_t evtid) const
{
    return finishedEventIds.find(evtid) != finishedEventIds.end();
}

Event* EventBuilder::makeNewEvent(const evtid_t evtid)
{
    Event newevt;
    newevt.SetLookupTable(lookupTable);
    eventCache.insert(evtid, std::move(newevt));
    return eventCache.get(evtid);
}

void EventBuilder::run()
{
    while (true) {
        // Get a raw frame from the input queue
        RawFrame raw;
        try {
            rawFrameQueue->get(raw);
        }
        catch (const NoMoreTasks&) {
            // There won't be more frames, so write all pending events to disk and return
            eventCache.flush();
            outputQueue->finish();
            return;
        }

        GRAWFrame frame (raw);  // Parse the raw frame
        evtid_t evtid = frame.eventId;

        // Try to get this event from the event cache
        Event* evtPtr = nullptr;
        try {
            evtPtr = eventCache.get(evtid);
        }
        catch (const std::out_of_range&) {
            // Event was not in the cache. Was it already evicted?
            if (eventWasAlreadyWritten(evtid)) {
                // This event was already evicted. This is a problem.
                std::cout << "Found frame for event " << evtid << ", but this event was already written!"
                          << std::endl;
                continue;
            }
            else {
                // This must be an event we haven't seen yet, so make a new one.
                evtPtr = makeNewEvent(evtid);
            }
        }
        assert(evtPtr != nullptr);

        evtPtr->AppendFrame(frame);
    }
}

void EventBuilder::processAndOutputEvent(Event&& evt)
{
    evt.SubtractFPN();
    finishedEventIds.emplace(evt.eventId);
    outputQueue->put(std::forward<Event&&>(evt));
}

void HDFWriter::run()
{
    while (true) {
        try {
            Event evt;
            eventQueue->get(evt);
            std::cout << "Event " << evt.eventId << " was written" << std::endl;
            hfile.writeEvent(evt);
        }
        catch (const NoMoreTasks&) {
            return;
        }
        catch (std::exception& e) {
            std::cout << "Error in writer: " << e.what() << std::endl;
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
