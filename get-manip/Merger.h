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

#include <map>
#include <vector>
#include <queue>
#include <boost/filesystem.hpp>
#include <memory>
#include <string>
#include <iostream>

class Merger
{
public:
    Merger();
    int AddFramesFromFileToIndex(const boost::filesystem::path& fpath);
    void MergeByEvtId(const std::string& outfilename, PadLookupTable* lt);
    
private:
    struct FrameMetadata
    {
        std::shared_ptr<GRAWFile> filePtr;
        size_t filePos; // Position of the frame in the file
        uint32_t evtId;
        uint64_t evtTime;
    };
    
    typedef std::multimap<uint32_t, FrameMetadata> MergingMap;
    MergingMap mmap;
    
    std::map<std::string, std::shared_ptr<GRAWFile>> files;
    
    FrameMetadata ParseRawFrame (const std::vector<uint8_t> raw);
    
    void ShowProgress(int currEvt, int numEvt);
};

#endif /* defined(__get_manip__Merger__) */
