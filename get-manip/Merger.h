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
#include "EventFile.h"
#include "GMExceptions.h"
#include "PadLookupTable.h"
#include "Constants.h"

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

/** \brief GRAW file merger class
 
 This class implements the actual file merging functionality. It receives GRAW file paths from the caller, opens them, indexes their contents by event ID, and merges events by event ID into an output file. 
 
 The design of the merger was inspired by a similar program found in the cobo-frame-viewer code.
 
 */
class Merger
{
public:
    //! \brief The default constructor. Does nothing.
    Merger();
    
    /** \brief Opens and indexes the given file.
     
     This function opens the file found at the given path and adds its contents to the frame index. It does not explicitly verify that the file exists and is valid, so this should be checked ahead of time.
     
     \returns The number of frames found in the file.
     \throws A variety of exceptions since it uses the GRAWFrame class internally.
     \throws Exceptions::File_Already_Read if the given file is already in the index.
     
     */
    int AddFramesFromFileToIndex(const boost::filesystem::path& fpath);
    
    /** \brief Merges the indexed events by common event ID.
     
     This function does the actual merging of the files. It looks through the map of events created by the AddFramesFromFileToIndex function and puts together frames that have the same event ID. The fixed pattern noise is subtracted from these events, and they are then written to a new file.
     
     */
    void MergeByEvtId(const std::string& outfilename, PadLookupTable* lt,
                      LookupTable<sample_t>& pedsTable, bool suppZeros,
                      sample_t threshold);
    
private:
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
    
    using PTT = Event(void);
    
    template<typename T>
    class SyncQueue {
    public:
        void put(const T& task);
        void put(T&& task);
        void get(T& dest);
        
    private:
        std::mutex qmtx;
        std::condition_variable cond;
        std::list<T> q;
    };
    
    SyncQueue<std::packaged_task<PTT>> tq;
    SyncQueue<std::future<Event>> resq;
    
    void TaskWorker();
    void ResultWriter(EventFile& of);
    
    //! \brief Provides the position and file pointer for an event
    struct MergingMapEntry
    {
        std::shared_ptr<GRAWFile> filePtr;
        std::streamoff filePos;
    };
    
    /** \brief A map linking event ID to frames in GRAW files.
    
     The keys in the map represent event IDs, and the mapped values provide a file pointer and offset.
     */
    typedef std::multimap<evtid_t, MergingMapEntry> MergingMap;
    MergingMap mmap;
    
    // This map is for keeping track of what files we've already seen
    std::map<std::string, std::shared_ptr<GRAWFile>> files;

    //! \brief Creates the progress bar in the terminal
    void ShowProgress(int currEvt, int numEvt);
};

#endif /* defined(__get_manip__Merger__) */
