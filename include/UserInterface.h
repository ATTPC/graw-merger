#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <iostream>
#include <iomanip>
#include <string>
#include <assert.h>

#define LOG_ERROR   std::cerr << '\r' << std::string(40,' ') << '\r' << "Error: "
#define LOG_WARNING std::cerr << '\r' << std::string(40,' ') << '\r' << "Warning: "

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



#endif /* defined(USERINTERFACE_H) */
