//
//  Merger.h
//  get-manip
//
//  Created by Joshua Bradt on 9/19/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__Merger__
#define __get_manip__Merger__

#include "GRAWFile.h"
#include "GRAWFrame.h"
#include "GMExceptions.h"
#include "PadLookupTable.h"
#include "HDFDataStore.h"
#include "Constants.h"
#include "Event.h"
#include "SyncQueue.h"

#include <map>
#include <vector>
#include <queue>
#include <list>
#include <boost/filesystem.hpp>
#include <memory>
#include <mutex>
#include <future>
#include <thread>
#include <string>
#include <iostream>

using task_type = Event(void);
using taskQueue_type = SyncQueue<std::packaged_task<task_type>>;
using futureQueue_type = SyncQueue<std::future<Event>>;

/** \brief GRAW file merger class

 This class implements the actual file merging functionality. It receives GRAW file paths from the caller, opens them, indexes their contents by event ID, and merges events by event ID into an output file.

 The design of the merger was inspired by a similar program found in the cobo-frame-viewer code.

 */
 class EventProcessingTask {
 public:
     EventProcessingTask(std::queue<GRAWFrame> fr, PadLookupTable* lt, LookupTable<sample_t> peds, bool suppZeros, sample_t th);

     Event operator()();

 private:
     std::queue<GRAWFrame> frames;
     PadLookupTable* pads;
     LookupTable<sample_t> peds;
     bool suppZeros;
     sample_t threshold;
 };

class Merger
{
public:
    Merger();
    int AddFramesFromFileToIndex(const boost::filesystem::path& fpath);
    void MergeByEvtId(const std::string& outfilename, PadLookupTable* lt,
                      LookupTable<sample_t>& pedsTable, bool suppZeros,
                      sample_t threshold);

private:
    std::shared_ptr<taskQueue_type> tq;
    std::shared_ptr<futureQueue_type> resq;

    struct MergingMapEntry
    {
        std::shared_ptr<GRAWFile> filePtr;
        std::streamoff filePos;
    };

    typedef std::multimap<evtid_t, MergingMapEntry> MergingMap;
    MergingMap mmap;

    // This map is for keeping track of what files we've already seen
    std::map<std::string, std::shared_ptr<GRAWFile>> files;

    //! \brief Creates the progress bar in the terminal
    void ShowProgress(int currEvt, int numEvt);
};

class Worker
{
public:
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

class TaskWorker : public Worker
{
public:
    TaskWorker(const std::shared_ptr<taskQueue_type>& inputQueue,
               const std::shared_ptr<futureQueue_type>& outputQueue)
    : inq(inputQueue), outq(outputQueue) {}

    void run() override;

private:
    std::shared_ptr<taskQueue_type> inq;
    std::shared_ptr<futureQueue_type> outq;
};

class HDFWriterWorker : public Worker
{
public:
    HDFWriterWorker(const std::string& filePath, const std::shared_ptr<futureQueue_type>& outputQueue)
    : hfile(filePath, true), futureq(outputQueue) {}

    void run() override;

private:
    HDFDataStore hfile;
    std::shared_ptr<futureQueue_type> futureq;
};

#endif /* defined(__get_manip__Merger__) */
