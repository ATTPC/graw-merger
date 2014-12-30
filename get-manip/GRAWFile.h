//
//  GRAWFile.h
//  get-manip
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__GRAWFile__
#define __get_manip__GRAWFile__

#include <iostream>
#include <fstream>
#include <vector>
#include <boost/filesystem.hpp>
#include <exception>
#include <string>
#include <cmath>

#include "GMExceptions.h"
#include "Utilities.h"
#include "DataFile.h"
#include "GRAWFrame.h"
#include "Constants.h"

/** \brief Interface to a .GRAW file.
 
     This class represents the .GRAW file output by the DAQ. It contains a
     C++ filestream to read in the file from disk, and it can then read
     individual frames from the file and provide them to another object, such
     as an instance of the GRAWFrame class.
 */
class GRAWFile : public DataFile
{
    
public:
    /** \brief Default constructor
     
     Performs no initialization. If you use this, you must call one of the other file-opening functions afterwards to initialize the file object.
     */
    GRAWFile();
    
    /** \brief String constructor
     
     Constructs the object and immediately opens the file found at the provided path.
     
     \param path The path to the file.
     \param mode A standard open mode for the file, either std::ios::in or std::ios::out.
     */
    GRAWFile(const std::string& path, const std::ios::openmode mode);
    
    /**  \overload */
    GRAWFile(const boost::filesystem::path& path, const std::ios::openmode mode);
    
    /** \brief Opens a file for input
        
     This can only be used if the object is not already initialized. This function also extracts the CoBo and AsAd number from the file path, so it is important that the directory tree is in the expected format.
     
     \param path The path to the file.
     */
    void OpenFileForRead(const std::string& path) override;
    
    //! \overload
    void OpenFileForRead(const boost::filesystem::path& path) override;
    
    /** \brief Open a file for output
     
     This can only be used if the object is not already initialized.
     
     \param path The path to the file.
     */
    void OpenFileForWrite(const std::string& path) override;
    
    //! \overload
    void OpenFileForWrite(const boost::filesystem::path& path) override;
    
    //! \brief Find the size of the next frame in the file
    uint16_t GetNextFrameSize();
    
    /** \brief Get the next raw frame from the file
     
     This function returns the next raw frame from the data file as a vector of integers. This can then be processed using, say, a GRAWFrame. This data is big-endian by default.
     
     \return A vector of unprocessed, big-endian bytes.
     
     \throws Exceptions::Bad_File Thrown if the file is not good.
     
     \throws Exceptions::Frame_Read_Error Thrown if the read frame has size zero.
     */
    std::vector<uint8_t> ReadRawFrame() override;
    
    /** \brief A structure containing metadata describing a frame.
     
     This struct is useful when a function only needs some of the header information from a frame, as it can be read much faster than an entire frame. This is used, for example, in the file indexing function of the merger.
     */
    struct FrameMetadata
    {
        //! \brief Position of the frame in the GRAW file
        std::streamoff filePos;
        
        //! \brief The event ID from the header
        evtid_t evtId;
        
        //! \brief The event time from the header
        ts_t evtTime;
    };
    
    /** \brief Read some metadata from the header of the next frame.
     
     This function reads the header of the next frame in the file and extracts the information contained in the :class:`FrameMetadata` struct. This leaves the filestream position at the start of the next frame.
     
     \return FrameMetadata struct describing the frame
     */
    FrameMetadata ReadFrameMetadata();

    /** \brief Writes a frame to the file.
     
     The file must be opened for output for this function to work properly.
     
     \param frame The frame to write.
     
     */
    void WriteFrame(const GRAWFrame& frame);
    
    //! \brief Returns the CoBo number from the file path
    virtual addr_t GetFileCobo() const;
    
    //! \brief Returns the AsAd number from the filename
    virtual addr_t GetFileAsad() const;
    
    //! \brief Returns the event number of the next frame in the file.
    //! \throws Exceptions::End_of_File if there is not another frame.
    virtual const evtid_t NextFrameEvtId();
    
private:
    
    addr_t coboId;                      // CoBo ID from the file name
    addr_t asadId;                      // AsAd ID from the file name
    
    template<typename T>
    static void AppendBytes(std::vector<uint8_t>& vec, T val, int nBytes);
    
    friend class Merger;
};

#endif /* defined(__get_manip__GRAWFile__) */
