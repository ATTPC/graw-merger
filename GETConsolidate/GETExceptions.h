//
//  GETExceptions.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__TextException__
#define __GETConsolidate__TextException__

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
    class GenericException : public std::exception
    {
    private:
        const char* reasonString;
        
    public:
        //! \param reason The reason for the exception.
        GenericException(const char* reason) : reasonString(reason) {};
        
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

    class Does_Not_Exist : public std::exception
    {
    private:
        std::string msg {"File/Directory does not exist: "};
        
    public:
        Does_Not_Exist(const char* filename_in) {msg.append(filename_in);};
        Does_Not_Exist(const std::string& filename_in) {msg.append(filename_in);};
        virtual const char* what() const noexcept {return msg.c_str();};
    };

    class Dir_is_Empty : public std::exception
    {
    private:
        std::string msg {"This directory is empty: "};
        
    public:
        Dir_is_Empty(const char* filename_in) {msg.append(filename_in);};
        Dir_is_Empty(const std::string& filename_in) {msg.append(filename_in);};
        virtual const char* what() const noexcept {return msg.c_str();};
    };

    class Frame_Read_Error : public std::exception
    {
    private:
        std::string msg {"Frame read failed."};
        
    public:
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    class Wrong_File_Position : public std::exception
    {
    private:
        std::string msg {"Item not found at this file position."};
        
    public:
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    class Bad_Data : public std::exception
    {
    private:
        std::string msg {"Corrupted or invalid data encountered."};
        
    public:
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    class End_of_File : public std::exception
    {
    private:
        std::string msg {"Reached end of file."};
        
    public:
        virtual const char* what() const noexcept {return msg.c_str();};
    };
}


#endif /* defined(__GETConsolidate__TextException__) */
