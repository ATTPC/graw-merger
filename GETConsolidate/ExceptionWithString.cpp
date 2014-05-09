//
//  ExceptionWithString.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/9/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "ExceptionWithString.h"

const char* ExceptionWithString::what() const throw()
{
    return reasonString;
}