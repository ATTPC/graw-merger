//
//  ExceptionWithString.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__TextException__
#define __GETConsolidate__TextException__

#include <iostream>
#include <exception>

class ExceptionWithString : public std::exception
{
private:
    const char* reasonString;
    
public:
    ExceptionWithString(const char* reason) : reasonString(reason) {};
    virtual const char* what() const throw();
};

#endif /* defined(__GETConsolidate__TextException__) */
