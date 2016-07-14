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
    BOOST_LOG_TRIVIAL(info) << "Beginning merge";

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
            catch (const std::exception& err) {
                BOOST_LOG_TRIVIAL(error) << "Error reading " << file->GetFilename() << ": " << err.what()
                                         << ". File will be closed.";

                // Find the file in *our* list and erase it
                auto doneFileIter = std::find(files.begin(), files.end(), file);
                (*doneFileIter)->CloseFile();  // This should prevent findFilesForEvtId from returning this file again
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
                BOOST_LOG_TRIVIAL(warning) << "Found frame for event " << evtid << ", but this event was already written!";
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
            BOOST_LOG_TRIVIAL(trace) << "Event " << evt.eventId << " was written";
            hfile.writeEvent(evt);
            numEvtsWritten++;
            if (numEvtsWritten % 100 == 0) {
                BOOST_LOG_TRIVIAL(info) << numEvtsWritten << " events have been written";
            }
        }
        catch (const NoMoreTasks&) {
            return;
        }
        catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "Error in writer: " << e.what() << std::endl;
        }
    }
}
