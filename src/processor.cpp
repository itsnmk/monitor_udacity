
#include "processor.h"

#include "linux_parser.h"
// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  Idle = LinuxParser::IdleJiffies();
  NonIdle = LinuxParser::ActiveJiffies();
  long prevTotal = prevIdle + prevNonIdle;
  long Total = Idle + NonIdle;

  long TotalDelta = Total - prevTotal;
  long IdleDelta = Idle - prevIdle;
  prevIdle = Idle;
  prevNonIdle = NonIdle;
  return (double)(TotalDelta - IdleDelta) / TotalDelta;
}


