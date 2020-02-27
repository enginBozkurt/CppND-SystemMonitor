#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>

#include "process.h"
#include "processor.h"

using std::string;

class System {
 public:
  Processor& Cpu();
  std::vector<Process>& Processes();
  float MemoryUtilization();
  long UpTime();
  int TotalProcesses();
  int RunningProcesses();
  std::string Kernel();
  std::string OperatingSystem();

 private:
  Processor cpu_;
  std::vector<Process> processes_;
  bool static compareProcesses(Process p, Process q);
};
#endif