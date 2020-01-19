#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::cout;

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
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string mem_total = "MemTotal:";
  string mem_free = "MemFree:";
  float total = 0.0;
  string line, name;
  float size;
  
  int total_memory = 0;
  int free_memory = 0;
  
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (filestream >> name >> size) {
      if (name.compare(mem_total) == 0) {
        total_memory = size;
      }
      if (name.compare(mem_free) == 0) {
        free_memory = size;
      }
    }
   	total =  ((total_memory - free_memory) / total_memory);
  }
  return total;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  long uptime = 0;
  string up_time, down_time;
  std::ifstream file_stream(kProcDirectory + kUptimeFilename);
  if(file_stream.is_open()) {
    while(file_stream >> up_time >> down_time) {
      uptime = stol(up_time);
    }
  }
  return uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpus = LinuxParser::CpuUtilization();
  return (
    stof(cpus[kUser_]) +
    stof(cpus[kNice_]) +
    stof(cpus[kSystem_]) +
    stof(cpus[kIRQ_]) +
    stof(cpus[kSoftIRQ_]) +
    stof(cpus[kSteal_]) +
    stof(cpus[kGuest_]) +
    stof(cpus[kGuestNice_])
  );
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpus = LinuxParser::CpuUtilization();
  return (stof(cpus[kIdle_]) + stof(cpus[kIOwait_]));
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  std::string cpu = "cpu";
  std::string line;
  std::ifstream file(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
  if (file.is_open()) {
    while(getline(file, line)) {
      if (line.compare(0, cpu.size(), cpu) == 0) {
        std::istringstream buffer(line);
        std::istream_iterator<string> begin(buffer), end;
        vector<string> line_content(begin, end);
        return line_content;
      }
    }
  }
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  int result = 0;
  std::string processes = "processes";
  std::string line;
  std::ifstream file(kProcDirectory + kStatFilename);
  if (file) {
    while(getline(file, line)) {
      std::stringstream ss(line);
      std::string sl, count;
      while(ss >> sl >> count) {
        if (sl.compare(processes) == 0) {
          result = stoi(count);
        }
      }
    }
  }
  return result;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  int result = 0;
  std::string running_processes = "procs_running";
  std::string line;
  std::ifstream file(kProcDirectory + kStatFilename);
  if (file) {
    while(getline(file, line)) {
      std::stringstream ss(line);
      std::string sl, count;
      while(ss >> sl >> count) {
        if (sl.compare(running_processes) == 0) {
          result = stoi(count);
        }
      }
    }
  }
  return result;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  std::string line;
  std::ifstream file(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if(file) {
    std::getline(file, line);
  }
  return line;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  std::string name;
  std::string size;
  std::string ram = "VmSize:";
  int in_megaByte = 0;
  std::ifstream file(kProcDirectory + to_string(pid) + kStatusFilename);
  if (file) {
    while (file >> name >> size) {
      if (name.compare(ram) == 0) {
         in_megaByte = static_cast<int>(stoi(size) / 1000);
      }
    }
  }
   return to_string(in_megaByte);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  std::string uid;
  std::string line;
  std::string id = to_string(pid);
  std::ifstream file(kPasswordPath);
  if(file) {
    while(getline(file, line)) {
      std::string delimiter = ":x:";
      size_t initPos = 0;
      size_t delimiterPos = line.find(delimiter);
      string userId = line.substr(delimiterPos + 3, id.size());
      if (userId.compare(id) == 0) {
        uid = line.substr(initPos, delimiterPos);
      }
    }
  }
  return uid;
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  std::string name;
  std::string user;
  std::string uid = "Uid:";
  std::ifstream file(kProcDirectory + to_string(pid) + kStatFilename);
  if(file) {
    while(file >> name >> user) {
      if (name.compare(uid) == 0) {
        return user;
      }
    }
  }
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  std::string uptime;
  std::ifstream file(kProcDirectory + to_string(pid) + kStatFilename);
  if(file.is_open()) { 
    file >> uptime;
  }
  return stol(uptime);
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
float LinuxParser::CpuUtilization(int pid) {
  int uptime, utime, stime, cutime, cstime, starttime;
  string line;
  std::ifstream file_stream(kProcDirectory + to_string(pid) + kStatFilename);
  if(file_stream.is_open()) { 
    getline(file_stream, line);
    std::istringstream ss(line);
    std::string s;
    int count = 0;
    while(ss >> s) {
      count++;
      if (count == 1) uptime = stoi(s);
      if (count == 14) utime = stoi(s);
      if (count == 15) stime = stoi(s);
      if (count == 16) cutime = stoi(s);
      if (count == 17) cstime = stoi(s);
      if (count == 22) starttime = stoi(s);
    }
    float total_time = utime + stime;
    // Calculate children processes.
    total_time = total_time + cutime + cstime;
    float seconds = uptime - (starttime / sysconf(_SC_CLK_TCK));
    float cpu_usage = (total_time / sysconf(_SC_CLK_TCK)) / seconds;
    return cpu_usage;
  }
}