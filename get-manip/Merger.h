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

#include <unordered_map>
#include <vector>
#include <boost/filesystem.hpp>
#include <memory>
#include <string>
#include <iostream>

class Merger
{
public:
    Merger();
    int AddFramesFromFileToIndex(boost::filesystem::path fpath);
    
    
private:
    struct FrameMetadata
    {
        std::shared_ptr<GRAWFile> filePtr;
        size_t filePos; // Position of the frame in the file
        uint32_t evtId;
        uint64_t evtTime;
    };
    
    typedef std::unordered_multimap<uint32_t, FrameMetadata> MergingMap;
    MergingMap mmap;
    
    std::unordered_map<std::string, std::shared_ptr<GRAWFile>> files;
    
    FrameMetadata ParseRawFrame (const std::vector<uint8_t> raw);
};

#endif /* defined(__get_manip__Merger__) */
