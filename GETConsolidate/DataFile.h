//
//  DataFile.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/19/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__DataFile__
#define __GETConsolidate__DataFile__

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <string>
#include <map>
#include <vector>

#include "GETExceptions.h"

/** \brief A generic class representing a data file.
 
 This class represents a binary data file on disk. It cannot be used directly, but should be inherited from. It provides basic functionality like opening and closing files, and checking if files are at EOF. The method ReadRawFrame must be overridden with an appropriate method for reading from the file.
 
 */
class DataFile
{
public:
    /** \brief The default constructor.
     
     This constructs a valid object, but it does not set a file path. Therefore, the object is left in an uninitialized state, and cannot be used yet. To use it after construction, call one of the OpenFile methods.
     
     */
    DataFile();
    
    /** \brief Construct the object and open a file.
     
     \rst
     
     This constructor constructs the object and opens the file located at the supplied path. Internally, it simply calls one of the OpenFile methods, so any exceptions that they throw can be thrown by this constructor as well. The second argument takes a standard open mode from the `fstream` library. This could be 
     
     *  `std::ios::in` --- to open a file for reading
     
     *  `std::ios::out` --- to open a file for writing
     
     Only those two options are supported. The file is always opened in binary mode.
     
     \endrst
     
     \param path The path to the file or output location.
     \param mode The open mode for the file (in or out).
     
     */
    DataFile(const std::string& path, const std::ios::openmode mode);
    
    //! \overload
    DataFile(const boost::filesystem::path& path, const std::ios::openmode mode);
    
    virtual void OpenFileForWrite(const std::string& path);
    virtual void OpenFileForWrite(const boost::filesystem::path& path);
    
    virtual void OpenFileForRead(const std::string& path);
    virtual void OpenFileForRead(const boost::filesystem::path& path);
    
    virtual void CloseFile();
    
    virtual std::vector<uint8_t> ReadRawFrame() = 0;
    
    virtual bool eof() const;
    
protected:
    boost::filesystem::path filePath;
    std::fstream filestream;
    
    bool isInitialized = false;
    bool isEOF = false;
};

#endif /* defined(__GETConsolidate__DataFile__) */
