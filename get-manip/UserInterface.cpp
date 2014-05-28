//
//  UserInterface.cpp
//  GETConsolidate
//
//  Created by Joshua Bradt on 5/25/14.
//  Copyright (c) 2014 NSCL. All rights reserved.
//

#include "UserInterface.h"

int UI::ProgressBar::width = 27;

UI::ProgressBar::ProgressBar()
: pct(0)
{}

UI::ProgressBar::ProgressBar(int i)
{
    assert(i < 100 || i > 0);
    pct = i;
}

void UI::ProgressBar::Write()
{
	std::cout << '\r' << std::string(22,' ') << '\r';
	std::cout << std::setw(3) << pct << "% ";
	std::cout << '[' << std::string(2*pct/10,'=') << std::string(20-2*pct/10,' ') << ']';
	std::cout.flush();
}

void UI::ProgressBar::SetPercent(int pct_in)
{
    assert(pct_in < 100 || pct_in >= 0);
	pct = pct_in;
}

int UI::ProgressBar::GetPercent(int pct_in)
{
    return pct;
}

int UI::ProgressBar::GetWidth()
{
    return width;
}

// --------
// Logger Class
// --------

UI::LogLevel UI::Logger::currentLogLevel = UI::LogLevel::Info;

//void UI::Log(LogLevel lvl, const char* msg)
//{
//    if (currentLogLevel >= lvl) {
//        // Cover over progress bar
//        std::cout << '\r' << std::string(ProgressBar::width,' ') << '\r';
//        
//        switch (lvl) {
//            case LogLevel::Error:
//                std::cout << "Error: ";
//                break;
//            case LogLevel::Warning:
//                std::cout << "Warning: ";
//                break;
//            case LogLevel::Info:
//                std::cout << "Info: ";
//                break;
//            case LogLevel::Debug:
//                std::cout << "Debug: ";
//                break;
//            default:
//                break;
//        }
//        
//        std::cout << msg << std::endl;
//    }
//}

void UI::Logger::SetLogLevel(LogLevel lvl)
{
    currentLogLevel = lvl;
}