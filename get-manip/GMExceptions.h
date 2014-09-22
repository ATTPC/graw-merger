//
//  GMExceptions.h
//  get-manip
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __get_manip__TextException__
#define __get_manip__TextException__

#include <iostream>
#include <exception>
#include <string>

/** \brief Custom exceptions
 
 This namespace contains a variety of exceptions that can be thrown by various functions within this code. All of them can return an error message that can be printed.
 */
namespace Exceptions {

    /** \brief A nonspecific exception.
    
    This should be used sparingly.
     */
    class Generic_Exception : public std::exception
    {
    private:
        const char* reasonString;
        
    public:
        //! \param reason The reason for the exception.
        Generic_Exception(const char* reason) : reasonString(reason) {};
        
        //! \returns The string provided on construction.
        virtual const char* what() const noexcept;
    };

    /** \brief An object was not initialized.
     
     This is thrown when the program attempts to read data from an object that needs to be initialized before use. This can happen with, for example, any of the classes that represent interfaces to files (specifically, those derived from DataFile).
     
     */
    class Not_Init : public std::exception
    {
    public:
        //! \return The string "Object not initialized."
        virtual const char* what() const noexcept {return "Object not initialized.";};
    };

    /** \brief The object is already initialized.
     
     This is thrown when the program attempts to initialize an object that has already been initialized. This is thrown, for example, by any of the file classes derived from DataFile if you attempt to open a file when there is already a file open.
     
     */
    class Already_Init : public std::exception
    {
    public:
        //! \return The string "Object already initialized."
        virtual const char* what() const noexcept {return "Object already initialized.";};
    };

    //! \brief The program encountered an invalid file.
    class Bad_File : public std::exception
    {
    private:
        std::string msg {"Bad file: "};
        
    public:
        //! \param filename_in The name of the file.
        Bad_File(const char* filename_in) {msg.append(filename_in);};
        
        //! \overload
        Bad_File(const std::string& filename_in) {msg.append(filename_in);};
        
        //! \return The string "Bad file: [filename]".
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    //! \brief The program could not open the specified file.
    class File_Open_Failed : public std::exception
    {
    private:
        std::string msg {"Failed to open file: "};
        
    public:
        //! \param filename_in The name of the file.
        File_Open_Failed(const char* filename_in) {msg.append(filename_in);};
        
        //! \overload
        File_Open_Failed(const std::string filename_in) {msg.append(filename_in);};
        
        //! \return The string "Failed to open file: [filename]"
        virtual const char* what() const noexcept {return msg.c_str();};
    };

    /** \brief The provided file was of the wrong kind.
     
     \rst
     
     This can happen for any of the file types the program recognizes.
     
     *  For a GRAW file, this most likely indicates that the extension on the file is wrong.
     
     *  For an Event file, this is generally thrown if the file's magic number is not correct. See the document :doc:`event_file` for more information about the magic number.
     
     \endrst
     
     */
    class Wrong_File_Type : public std::exception
    {
    private:
        std::string msg {"Wrong file type: "};
        
    public:
        //! \param filename_in The name of the file.
        Wrong_File_Type(const char* filename_in) {msg.append(filename_in);};
        
        //! \overload
        Wrong_File_Type(const std::string filename_in) {msg.append(filename_in);};
        
        //! \return The string "Wrong file type: [filename]"
        virtual const char* what() const noexcept {return msg.c_str();};
    };

    //! \brief The file or directory provided does not exist.
    class Does_Not_Exist : public std::exception
    {
    private:
        std::string msg {"File/Directory does not exist: "};
        
    public:
        //! \param filename_in The filename or path.
        Does_Not_Exist(const char* filename_in) {msg.append(filename_in);};
        
        //! \overload
        Does_Not_Exist(const std::string& filename_in) {msg.append(filename_in);};
        
        //! \return The string "File/Directory does not exist: [filename]"
        virtual const char* what() const noexcept {return msg.c_str();};
    };

    //! \brief The provided directory had no files in it.
    class Dir_is_Empty : public std::exception
    {
    private:
        std::string msg {"This directory is empty: "};
        
    public:
        //! \param filename_in The name of the directory.
        Dir_is_Empty(const char* filename_in) {msg.append(filename_in);};
        
        //! \overload
        Dir_is_Empty(const std::string& filename_in) {msg.append(filename_in);};
        
        //! \return The string "This directory is empty: [directory]"
        virtual const char* what() const noexcept {return msg.c_str();};
    };

    /** \brief The program failed to read a frame from a file.
     
     This could be thrown by any of the file interface classes. Most often, it is thrown when an attempt to read a raw frame from a file yields a vector of size 0. This might indicate that the file pointer is at the end of the file, though Exceptions::End_of_File should be thrown instead.
     
     */
    class Frame_Read_Error : public std::exception
    {
    private:
        std::string msg {"Frame read failed."};
        
    public:
        //! \return The string "Frame read failed."
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    /** \brief Could not find the required item at that file position.
     
     This exception is thrown when the program attempts to read an item like a GRAWFrame or an Event from a file, but it doesn't seem like such an item exists at the current file location. This could mean that the file cursor has gotten out-of-position in the binary file (i.e. it isn't aligned to the beginning of an event or frame), and unfortunately, there's not much of a way to recover from this.
     
     */
    class Wrong_File_Position : public std::exception
    {
    private:
        std::string msg {"Item not found at this file position."};
        
    public:
        //! \return The string "Item not found at this file position."
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    /** \brief Invalid data was read from the file.
     
     This is thrown if the input cannot be interpreted. This might indicate that the CoBo, AsAd, AGET, or channel number is outside its expected range of acceptable values.
     
     */
    class Bad_Data : public std::exception
    {
    private:
        std::string msg {"Corrupted or invalid data encountered."};
        
    public:
        //! \return The string "Corrupted or invalid data encountered."
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    //! \brief The end of the file was encountered.
    class End_of_File : public std::exception
    {
    private:
        std::string msg {"Reached end of file."};
        
    public:
        //! \return "Reached end of file."
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    //! \brief The file was already read.
    class File_Already_Read : public std::exception
    {
    private:
        std::string msg {"This file has already been read: "};
        
    public:
        //! \param filename_in The name of the directory.
        File_Already_Read(const char* filename_in) {msg.append(filename_in);};
        
        //! \overload
        File_Already_Read(const std::string& filename_in) {msg.append(filename_in);};
        
        //! \return The string "This file has already been read: [filename]"
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    /** \brief Attempted to operate on an empty container.
     
     Thrown when a mathematical function is called on an empty container class, such as a Trace.
     
     */
    class No_Data : public std::exception
    {
    private:
        std::string msg {"No data in container."};
        
    public:
        //! \return The string "No data in container."
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    /** \brief Frames were out of order in a file
     
     This indicates that the program found a frame with a lower event index than the current one. This shouldn't normally happen.
     
     */
    class Frame_Out_of_Order : public std::exception
    {
    private:
        std::string msg {"Frame out of order in file: "};
        
    public:
        //! \param filename_in The filename or path.
        Frame_Out_of_Order(const char* filename_in) {msg.append(filename_in);};
        
        //! \overload
        Frame_Out_of_Order(const std::string& filename_in) {msg.append(filename_in);};
        
        //! \return The error message.
        virtual const char* what() const noexcept {return msg.c_str();};
    };
}


#endif /* defined(__get_manip__TextException__) */
