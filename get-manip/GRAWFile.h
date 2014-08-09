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
    
    /** \brief Get the next raw frame from the file
     
     This function returns the next raw frame from the data file as a vector of integers. This can then be processed using, say, a GRAWFrame. This data is big-endian by default.
     
     \return A vector of unprocessed, big-endian bytes.
     
     \throws Exceptions::Bad_File Thrown if the file is not good.
     
     \throws Exceptions::Frame_Read_Error Thrown if the read frame has size zero.
     */
    std::vector<uint8_t> ReadRawFrame() override;

    /** \brief Writes a frame to the file.
     
     The file must be opened for output for this function to work properly.
     
     \param frame The frame to write.
     
     */
    void WriteFrame(const GRAWFrame& frame);
    
    //! \brief Returns the CoBo number from the file path
    virtual uint8_t GetFileCobo() const;
    
    //! \brief Returns the AsAd number from the filename
    virtual uint8_t GetFileAsad() const;
    
    //! \brief Returns the event number of the next frame in the file.
    //! \throws Exceptions::End_of_File if there is not another frame.
    virtual const uint64_t NextFrameEvtId();
    
private:
    
    uint8_t coboId;                      // CoBo ID from the file name
    uint8_t asadId;                      // AsAd ID from the file name
    
    template<typename T>
    static void AppendBytes(std::vector<uint8_t>& vec, T val, int nBytes);
};

#endif /* defined(__get_manip__GRAWFile__) */
