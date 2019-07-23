#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <iostream>
#include <string>
#include <sys/time.h>      

#define INFO 0
#define DEBUG 1
#define WARNING 2
#define ERROR 3

uint64_t GetTimeStamp()
{
  struct timeval time_;
  gettimeofday(&time_, NULL);          //gettimeofday(timval *tv,timzone *tz) //  第二个参数和时区有关系，不关心可以赋NULL
  return time_.tv_sec;

}

std::string GetLogLevel (int level_)
{
  switch(level_){
    case 0:
      return "INFO";
    case 1:
      return "DEBUG";
    case 2:
      return "WARNING";
    case 3:
      return "ERROR";
    default:
      return "UNKNOW";

  }

}

void Log(int level_, std::string message_, std::string file_, int line_)
{
  std::cout << "[ " << GetTimeStamp() << "  ]" << " [ " << GetLogLevel(level_) << "  ]" << " [ " << file_ << " : " << line_ << "  ] " << message_ << std::endl;

}

#define LOG(level_, message_) Log(level_, message_, __FILE__, __LINE__)

// LOG(INFO, "bind success");
//
 #endif



