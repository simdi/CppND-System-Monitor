#include <string>
#include <iostream>

#include "processor.h"
#include "linux_parser.h"

using std::cout;

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  float totalTime = LinuxParser::Jiffies();
  float activeTime = LinuxParser::ActiveJiffies() ;
  float result = 1.0 * (activeTime/totalTime);
  return result;
}