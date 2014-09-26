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

int Merger::AddFramesFromFileToIndex(const boost::filesystem::path& fpath)
{
    if (files.find(fpath.filename().string()) != files.end()) {
        throw Exceptions::File_Already_Read(fpath.filename().string());
    }
    
    std::shared_ptr<GRAWFile> file {new GRAWFile(fpath, std::ios::in)};
    
    int nFramesRead {0};
    
    while (!file->eof()) {
        try {
            size_t currentPos = file->filestream.tellg();
            GRAWFrame frame {file->ReadRawFrame()};
            
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

void Merger::MergeByEvtId(const std::string &outfilename, PadLookupTable* lt)
{
    EventFile outfile;
    outfile.OpenFileForWrite(outfilename);
    
    std::cout << "Beginning merge of " << mmap.size() << " frames." << std::endl;
    
    // Find first event
    uint32_t firstEvtId = mmap.lower_bound(0)->first;
    uint32_t numEvts = mmap.rbegin()->first - firstEvtId;
    
    // Create events and append to file
    for (uint32_t currentEvtId = firstEvtId ; mmap.size() > 0; currentEvtId++ )
    {
        // Find frames for this event
        auto currentRange = mmap.equal_range(currentEvtId);
        if (currentRange.first->first > currentEvtId) continue;
        
        // Extract frames from files
        std::queue<GRAWFrame> frames;
        for (auto frameRefPtr = currentRange.first; frameRefPtr != currentRange.second; frameRefPtr++) {
            auto& fref = frameRefPtr->second;
            fref.filePtr->filestream.seekg(fref.filePos);
            std::vector<uint8_t> rawFrame = fref.filePtr->ReadRawFrame();
            frames.push(GRAWFrame {rawFrame});
        }
        
        // Append frames to a new event
        Event evt;
        evt.SetLookupTable(lt);
        
        while (!frames.empty()) {
            evt.AppendFrame(frames.front());
            frames.pop();
        }
        
        evt.SubtractFPN();
        outfile.WriteEvent(evt);
        
        mmap.erase(currentEvtId);
        ShowProgress(currentEvtId - firstEvtId, numEvts);
    }
}

void Merger::ShowProgress(int currEvt, int numEvt)
{
    const int progWidth = 40;
    int pct = floor(float(currEvt) / float(numEvt) * 100);
    std::cout << '\r' << std::string(progWidth,' ') << '\r';
    std::cout << std::setw(3) << pct << "% ";
    std::cout << '[' << std::string(2*pct/10,'=') << std::string(20-2*pct/10,' ') << ']';
    std::cout << " (" << currEvt << "/" << numEvt << ")";
    std::cout.flush();
}