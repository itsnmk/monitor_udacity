#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;


float LinuxParser::Round(float var) {
  float value = (int)(var * 100 + .5); 
  return (float)value / 100; 
} 

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
  string os, kernel,version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel >> version;
  }
  return version;
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
    string line;
  string key;
  string value;
  long memTotal{0};
  long memFree{0};
  std::ifstream data(kProcDirectory + kMeminfoFilename);
  if (data.is_open()) {
    while (std::getline(data, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          memTotal = std::stol(value);
        }
        if (key == "MemFree") {
          memFree = std::stol(value);
        }
      }
    }
    data.close();
  }
  return 1.0 * (memTotal - memFree) / memTotal;
   }

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
  
  long systemUptime{0};
  string line;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> systemUptime;
    filestream.close();
  }
  return systemUptime;
  }

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
   }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) { 
  string line;
  long utime{0};
  long stime{0};
  long cutime{0};
  long cstime{0};
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    vector<string> parsedLine((std::istream_iterator<string>(linestream)), std::istream_iterator<string>());    
    utime = std::stol(parsedLine[13]);
    stime = std::stol(parsedLine[14]);
    cutime = std::stol(parsedLine[15]);
    cstime = std::stol(parsedLine[16]);
    filestream.close();
  }
  return utime + stime + cutime + cstime;}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  long sum{0};
  vector<string> cpu = CpuUtilization();
  sum += stol(cpu[LinuxParser::CPUStates::kUser_]); 
  sum += stol(cpu[LinuxParser::CPUStates::kNice_]); 
  sum += stol(cpu[LinuxParser::CPUStates::kSystem_]); 
  sum += stol(cpu[LinuxParser::CPUStates::kIRQ_]); 
  sum += stol(cpu[LinuxParser::CPUStates::kSoftIRQ_]); 
  sum += stol(cpu[LinuxParser::CPUStates::kSteal_]); 
  return sum;}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  long idle{0};
  long iowait{0};
  vector<string> cpu = LinuxParser::CpuUtilization();
  idle = stol(cpu[LinuxParser::CPUStates::kIdle_]);
  iowait = stol(cpu[LinuxParser::CPUStates::kIOwait_]);
  return idle + iowait;
  }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpuState {};
  std::string line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if(filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    std::vector<string> values((std::istream_iterator<string>(linestream)), std::istream_iterator<string>());
    values.erase(values.begin());
    cpuState = values;
    filestream.close();
  }
  return cpuState;
  }

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  int processes{0};
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          processes = stoi(value);
          break;
        }
      }
    }
    filestream.close();
  }
  return processes;
  }

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  int running{0};
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          running = stoi(value);
          break;
        }
      }
    }
    filestream.close();
  }
  return running;
   }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
   string line;
  string command{};
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    command = line;
    filestream.close();
  }
  return command;
   }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { 
   string line;
  string result{};
  string key;
  string value;
  string size;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          // ram = totalRam - stol(value);
          result = to_string(LinuxParser::Round(stol(value) / 1000));
          result.erase(result.begin()+(result.find('.') + 3), result.end());
          break;
        }
      }
    }
    filestream.close();
  }
  return result;
  }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
    string realUserId{};
  string line;
  string key;
  string firstValue;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::stringstream linestream(line);
      while (linestream >> key >> firstValue) {
        if (key == "Uid:") {
          // std::cout << firstValue << "\n";
          realUserId = firstValue;
          break;
        }
      }
    }
    filestream.close();
  }
  return realUserId; }

//  user associated with a process


string LinuxParser::User(int pid) {
  string line;

  // First three values in a line in /etc/passwd
  string username;
  string pass;
  string uid;

  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> username >> pass >> uid) {
        if (uid == LinuxParser::Uid(pid)) {
          return username;
        }
      }
    }
  }
  return string();
}

// the uptime of a process
long LinuxParser::UpTime(int pid) { 
  long sysTime = LinuxParser::UpTime();
  long startTime{0};
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    vector<string> parsedLine((std::istream_iterator<string>(linestream)), std::istream_iterator<string>());
    startTime = stol(parsedLine[21]);
    filestream.close();
  }
  return sysTime - startTime / sysconf(_SC_CLK_TCK);
  }

  float LinuxParser::CpuUtilization(int pid) {
  // /proc/[PID]/stat  returns data in clockticks
  // total_time = cutime + cstime
  // elapsedTime = uptime - (starttime / Hertz)
  // cpu_usage = 100 * ((total_time / Hertz) / elapsedTime)
  string line;
  int utilTime;
  int elapsedTime;
  float cpuUsage{0.0};
  const long int upTime{LinuxParser::Jiffies()};
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    vector<string> procStats((std::istream_iterator<string>(linestream)), std::istream_iterator <string>());
    utilTime = stoi(procStats[LinuxParser::ProcCPUStats::cuTime_]) + stoi(procStats[LinuxParser::ProcCPUStats::csTime_]);
    elapsedTime = upTime - stoi(procStats[LinuxParser::ProcCPUStats::startTime_]);
    cpuUsage = 1.0 * utilTime / elapsedTime;
    filestream.close();
  }
  return cpuUsage;
}


