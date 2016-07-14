#ifndef MERGER_H
#define MERGER_H

#include "GRAWFile.h"
#include "GRAWFrame.h"
#include "GMExceptions.h"
#include "PadLookupTable.h"
#include "HDFDataStore.h"
#include "Constants.h"
#include "Event.h"
#include "SyncQueue.h"
#include "RawFrame.h"
#include "LRUCache.h"
#include "FileIndex.h"

#include <map>
#include <vector>
#include <queue>
#include <unordered_set>
#include <list>
#include <boost/log/trivial.hpp>
#include <memory>
#include <mutex>
#include <future>
#include <thread>
#include <string>
#include <iostream>
#include <cassert>

class Merger
{
public:
    Merger(const std::vector<std::string>& filePaths, const std::shared_ptr<PadLookupTable>& lt);
    void MergeByEvtId(const std::string& outfilename);

private:
    std::shared_ptr<SyncQueue<RawFrame>> frameQueue;
    std::shared_ptr<SyncQueue<Event>> eventQueue;
    std::shared_ptr<PadLookupTable> lookupTable;
    std::vector<std::shared_ptr<GRAWFile>> files;

    FileIndex findex;

    //! \brief Creates the progress bar in the terminal
    void ShowProgress(uint64_t currEvt, uint64_t numEvt);
};

class Worker
{
public:
    Worker() = default;
    Worker(const Worker&) = delete;
    Worker(Worker&&) = default;

    virtual ~Worker()
    {
        if (thr.joinable()) {
            thr.join();
        }
    }

    virtual void run() = 0;
    virtual void start()
    {
        thr = std::thread ( [this]{ return run(); } );
    }
    virtual void join()
    {
        thr.join();
    }

private:
    std::thread thr;
};

class EventBuilder : public Worker
{
public:
    EventBuilder(const std::shared_ptr<SyncQueue<RawFrame>>& rawFrameQueue,
                 const std::shared_ptr<SyncQueue<Event>>& outputQueue,
                 const std::shared_ptr<PadLookupTable>& lookupTable)
    : rawFrameQueue(rawFrameQueue), outputQueue(outputQueue),
      eventCache(10, std::bind(&EventBuilder::processAndOutputEvent, this, std::placeholders::_1)),
      lookupTable(lookupTable) {}
    EventBuilder(EventBuilder&&) = default;
    virtual ~EventBuilder() = default;

    void run() override;
    Event* makeNewEvent(const evtid_t evtid);
    bool eventWasAlreadyWritten(const evtid_t evtid) const;
    void processAndOutputEvent(Event&& evt);

private:
    std::shared_ptr<SyncQueue<RawFrame>> rawFrameQueue;
    std::shared_ptr<SyncQueue<Event>> outputQueue;
    LRUCache<evtid_t, Event> eventCache;
    std::shared_ptr<PadLookupTable> lookupTable;
    std::unordered_set<evtid_t> finishedEventIds;
};

class HDFWriter : public Worker
{
public:
    HDFWriter(const std::string& filePath,
              const std::shared_ptr<SyncQueue<Event>>& outputQueue)
    : hfile(filePath, true), eventQueue(outputQueue), numEvtsWritten(0) {}
    virtual ~HDFWriter() = default;

    void run() override;

private:
    HDFDataStore hfile;
    std::shared_ptr<SyncQueue<Event>> eventQueue;
    evtid_t numEvtsWritten;
};

#endif /* defined(MERGER_H) */
