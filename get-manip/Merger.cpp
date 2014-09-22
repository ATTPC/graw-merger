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

int Merger::AddFramesFromFileToIndex(boost::filesystem::path fpath)
{
    if (files.find(fpath.filename().string()) != files.end()) {
        throw Exceptions::File_Already_Read(fpath.filename().string());
    }
    
    std::shared_ptr<GRAWFile> file {new GRAWFile(fpath, std::ios::in)};
    
    int nFramesRead {0};
    
    while (!file->eof()) {
        try {
            size_t currentPos = file->filestream.tellg();
            GRAWFrame frame {file->ReadRawFrame(), 0, 0};
            
            FrameMetadata fm;
            fm.filePtr = file;
            fm.filePos = currentPos;
            fm.evtId = frame.GetEventId();
            fm.evtTime = frame.GetEventTime();
            
            mmap.emplace(fm.evtId, fm);
            nFramesRead++;
        }
        catch (Exceptions::Frame_Read_Error& frErr) {
            std::cerr << frErr.what() << std::endl;
            continue;
        }
        catch (std::exception& e) {
            std::cout << "Exception: " << e.what() << std::endl;
            throw e;
        }
    }
    
    files.emplace(fpath.filename().string(),file);
    return nFramesRead;
}

