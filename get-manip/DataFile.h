//
//  DataFile.h
//  get-manip
//
//  Created by Joshua Bradt on 5/19/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__DataFile__
#define __get_manip__DataFile__

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <string>
#include <map>
#include <vector>

#include "GMExceptions.h"

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
    
    /** \brief Open a file for output
     
     This method opens the given file for output. If the file does not exist, it is created. If the file does exist, it is overwritten. This method can only be called on a DataFile object that does not currently have an open filestream associated with it. Most often, this would be an object created using the default constructor.
     
     \param path The path to the file.
     
     \throws Exceptions::File_Open_Failed Thrown if the filestream is not useable after trying to open the file.
      
     \throws Exceptions::Already_Init Thrown if the object already has an open data file associated with it.
     
     */
    virtual void OpenFileForWrite(const std::string& path);
    
    //! \overload
    virtual void OpenFileForWrite(const boost::filesystem::path& path);
    
    /** \brief Open a file for input
     
     This method opens the given file for input. This should perhaps be overridden by derived classes if they need to do anything more specific than open the file and make sure it is readable. This method can only be called on a DataFile object that does not currently have an open filestream associated with it. Most often, this would be an object created using the default constructor.
     
     \param path The path to the file.
     
     \throws Exceptions::Already_Init Thrown if the object already has an open data file associated with it.
     
     \throws Exceptions::Does_Not_Exist Thrown if the file cannot be found.
     
     \throws Exceptions::Wrong_File_Type Thrown if the provided path does not lead to a regular file.
     
     \throws Exceptions::Bad_File Thrown if the file is not readable after opening.
     
     */
    virtual void OpenFileForRead(const std::string& path);
    
    //! \overload
    virtual void OpenFileForRead(const boost::filesystem::path& path);
    
    /** \brief Close the currently open file.
     
     This is usually not necessary as the destructor will take care of this for you.
     
     */
    virtual void CloseFile();
    
    /** \brief Read the next chunk of raw data from the file.
     
     This method must be overridden by the derived class, and it should extract a raw frame, event, or whatever unit of raw data makes sense for the file. This would usually represent a serialized object that is represented in memory by a different class.
     
     */
    virtual std::vector<uint8_t> ReadRawFrame() = 0;
    
    //! \brief Check if the end of the file has been reached.
    virtual bool eof() const;
    
    //! \brief Returns the current file position.
    virtual const long unsigned int GetPosition();
    
    //! \brief Returns the filename.
    virtual const std::string GetFilename() const;
    
protected:
    /** \brief The path to the file.
     
     This is set by either the constructor or one of the OpenFile methods.
     
     */
    boost::filesystem::path filePath;
    
    //! \brief The file itself.
    std::fstream filestream;
    
    //! \brief A boolean value indicating if the object has been initialized by the constructor or an OpenFile method.
    bool isInitialized = false;
    
    //! \brief A boolean indicating if the end of the file was reached.
    bool isEOF = false;
};

#endif /* defined(__get_manip__DataFile__) */
