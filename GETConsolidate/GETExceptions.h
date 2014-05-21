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
        GenericException(const char* reason) : reasonString(reason) {};
        
        //! \returns The string provided on construction.
        virtual const char* what() const noexcept;
    };

    class Not_Init : public std::exception
    {
    public:
        virtual const char* what() const noexcept {return "Object not initialized.";};
    };

    class Already_Init : public std::exception
    {
    public:
        virtual const char* what() const noexcept {return "Object already initialized.";};
    };

    class Bad_File : public std::exception
    {
    private:
        std::string msg {"Bad file: "};
        
    public:
        Bad_File(const char* filename_in) {msg.append(filename_in);};
        Bad_File(const std::string& filename_in) {msg.append(filename_in);};
        virtual const char* what() const noexcept {return msg.c_str();};
    };
    
    class File_Open_Failed : public std::exception
    {
    private:
        std::string msg {"Failed to open file: "};
        
    public:
        File_Open_Failed(const char* filename_in) {msg.append(filename_in);};
        File_Open_Failed(const std::string filename_in) {msg.append(filename_in);};
        virtual const char* what() const noexcept {return msg.c_str();};
    };

    class Wrong_File_Type : public std::exception
    {
    private:
        std::string msg {"Wrong file type: "};
        
    public:
        Wrong_File_Type(const char* filename_in) {msg.append(filename_in);};
        Wrong_File_Type(const std::string filename_in) {msg.append(filename_in);};
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
