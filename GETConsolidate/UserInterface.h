//
//  UserInterface.h
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/25/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#ifndef __GETConsolidate__UserInterface__
#define __GETConsolidate__UserInterface__

#include <iostream>
#include <iomanip>
#include <string>
#include <assert.h>

namespace UI {
	class ProgressBar;
    
    enum class LogLevel {
        Error = 0,
        Warning = 1,
        Info = 2,
        Debug = 3
    };
    
    class Logger;
}

class UI::ProgressBar
{
public:
    ProgressBar();
    ProgressBar(int i);
    
	void Write();
	void SetPercent(int pct_in);
	int GetPercent(int pct_in);
    int GetWidth();
    
    static int width;
    
private:
	int pct;
};

class UI::Logger
{
public:
    void SetLogLevel(LogLevel lvl);
    
    friend std::ostream& operator<<(UI::Logger& log, const char* msg);
    
private:
    static LogLevel currentLogLevel;
};



#endif /* defined(__GETConsolidate__UserInterface__) */
