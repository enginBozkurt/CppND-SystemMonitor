#include "processor.h"
#include "linux_parser.h"

using namespace LinuxParser;

// UNTESTED: Return the aggregate CPU utilization
float Processor::Utilization() { 
  float cpu_usage = (float) ActiveJiffies() / Jiffies();
  return cpu_usage;
}