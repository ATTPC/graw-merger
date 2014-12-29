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
    // Check if the file has already been read
    if (files.find(fpath.filename().string()) != files.end()) {
        throw Exceptions::File_Already_Read(fpath.filename().string());
    }
    
    // Open the file
    std::shared_ptr<GRAWFile> file {new GRAWFile(fpath, std::ios::in)};
    
    int nFramesRead {0};
    
    // Index the frames in the file
    while (!file->eof()) {
        try {
            std::streamoff currentPos = file->filestream.tellg();
            
            // Reading the frame will advance the file pointer
            GRAWFrame frame {file->ReadRawFrame()};  // inefficient?
            
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
            throw;
        }
    }
    
    // Put the file in the list of files we've already read. This also copies
    // the shared pointer, which keeps the file around after this function
    // returns.
    
    files.emplace(fpath.filename().string(),file);
    return nFramesRead;
}

void Merger::MergeByEvtId(const std::string &outfilename, PadLookupTable* lt,
                          LookupTable<sample_t>& pedsTable, bool suppZeros,
                          sample_t threshold)
{
    EventFile outfile;
    outfile.OpenFileForWrite(outfilename);
    
    std::cout << "Beginning merge of " << mmap.size() << " frames." << std::endl;
    
    // Find first event and number of events
    uint32_t firstEvtId = mmap.lower_bound(0)->first;
    uint32_t numEvts = mmap.rbegin()->first - firstEvtId;
    
    // Create events and append to file
    for (uint32_t currentEvtId = firstEvtId ; mmap.size() > 0; currentEvtId++ )
    {
        // Find frames for this event. equal_range() returns a pair containing
        // iterators to the first and last frame with this event ID. If no frames
        // are found, they'll both point to the next highest event ID.
        auto currentRange = mmap.equal_range(currentEvtId);
        if (currentRange.first->first > currentEvtId) continue; // no frames found
        
        // Extract frames from files
        std::queue<GRAWFrame> frames;
        for (auto frameRefPtr = currentRange.first; frameRefPtr != currentRange.second; frameRefPtr++) {
            FrameMetadata& fref = frameRefPtr->second;
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
        
        if (not pedsTable.Empty()) {
            evt.SubtractPedestals(pedsTable);
        }
        
        if (threshold > Constants::min_sample) {
            evt.ApplyThreshold(threshold);
        }
        
        outfile.WriteEvent(evt);
        
        mmap.erase(currentEvtId);
        ShowProgress(currentEvtId - firstEvtId, numEvts);
    }
}

void Merger::ShowProgress(int currEvt, int numEvt)
{
    // Produces a progress bar like this:
    //  50% [==========          ] (50/100)
    // The last part is the current evt / num evts
    
    const int progWidth = 40;
    int pct = floor(float(currEvt) / float(numEvt) * 100);
    std::cout << '\r' << std::string(progWidth,' ') << '\r';
    std::cout << std::setw(3) << pct << "% ";
    std::cout << '[' << std::string(2*pct/10,'=') << std::string(20-2*pct/10,' ') << ']';
    std::cout << " (" << currEvt << "/" << numEvt << ")";
    std::cout.flush();
}