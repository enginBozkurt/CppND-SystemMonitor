#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  string placeholder;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> placeholder >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Return memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  int value;
  int mem_total;
  int mem_free;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") mem_total = value;
        if (key == "MemFree:") mem_free = value;
      }
    }
  }
  return (mem_total - mem_free) / (float)mem_total;
}

// Return the system uptime in seconds
long LinuxParser::UpTime() {
  string line;
  long up_time = 0;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::stringstream linestream(line);
    linestream >> up_time;
  }
  return up_time;
}

// Returns the total system ticks since boot
long LinuxParser::Jiffies() {
  string line;
  string key;
  long jiffies = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::stringstream linestream(line);
    int value;
    // sum the values from the first line of the file
    // user + nice + system + idle + iowait + irq + softirq + steal
    for (int i = 0; i < 9; ++i) {
      if (i == 0) {
        linestream >> key;
      } else {
        linestream >> value;
        jiffies += value;
      }
    }
  }
  return jiffies;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line;
  string placeholder;
  long jiffies = 0;
  long process_jiffies = 0;
  string kPidDirectory = '/' + std::to_string(pid);
  std::ifstream filestream(kProcDirectory + kPidDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (int token_id = 1; token_id <= 17; ++token_id) {
      if (token_id == ProcessCpuStates::kCstime ||
          token_id == ProcessCpuStates::kCutime ||
          token_id == ProcessCpuStates::kStime ||
          token_id == ProcessCpuStates::kUtime) {
        linestream >> jiffies;
        process_jiffies += jiffies;
      } else {
        linestream >> placeholder;
      }
    }
  }
  return process_jiffies;
}

// Return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return Jiffies() - IdleJiffies(); }

// Return the idle ticks since boot
long LinuxParser::IdleJiffies() {
  string line;
  string key;
  long idleJiffies = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::stringstream linestream(line);
    int value;
    // sum the iddle ticks
    // idle + iowait
    for (int i = 0; i < 6; ++i) {
      if (i == 0) {
        linestream >> key;
      } else if (i > 3) {
        linestream >> value;
        idleJiffies += value;
      } else {
        linestream >> value;
      }
    }
  }
  return idleJiffies;
}

// Return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  int processes = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "processes") {
          linestream >> processes;
          break;
        }
      }
    }
  }
  return processes;
}

// Return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int running_processes = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "procs_running") {
          linestream >> running_processes;
          break;
        }
      }
    }
  }
  return running_processes;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string cmd_line;
  string kPidDirectory = '/' + std::to_string(pid);
  std::ifstream filestream(kProcDirectory + kPidDirectory + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, cmd_line);
  }
  return cmd_line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line;
  string key;
  long ram;
  string kPidDirectory = '/' + std::to_string(pid);
  std::ifstream filestream(kProcDirectory + kPidDirectory + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize:") {
        linestream >> ram;
        break;
      }
    }
  }
  return std::to_string(ram / 1000);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string uid;
  string kPidDirectory = '/' + std::to_string(pid);
  std::ifstream filestream(kProcDirectory + kPidDirectory + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "Uid:") {
          linestream >> uid;
          break;
        }
      }
    }
  }
  return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line;
  string user;
  string uid;
  string placeholder;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> user >> placeholder >> uid;
      if (uid == Uid(pid)) {
        break;
      }
    }
  }
  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line;
  string placeholder;
  long start_time = 0;
  string kPidDirectory = '/' + std::to_string(pid);
  std::ifstream filestream(kProcDirectory + kPidDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (int token_id = 1; token_id <= 22; ++token_id) {
      if (token_id == ProcessCpuStates::kStarttime) {
        linestream >> start_time;
      } else {
        linestream >> placeholder;
      }
    }
  }
  return start_time / sysconf(_SC_CLK_TCK);
}