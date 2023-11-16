#include "Timer.h"

//------------------------------------------------------------------------------
//    Constructor
//------------------------------------------------------------------------------
Timer::Timer(const bool _running) : running(_running),
                                    accumulatedCpuTime(0),
                                    accumulatedWallTime(0),
                                    startCpuTime(running ? get_cpu_time() : 0),
                                    startWallTime(running ? get_wall_time() : 0)
{}


//------------------------------------------------------------------------------
// Finds and returns the current time.
//------------------------------------------------------------------------------
tm *Timer::current_time() const
{
  time_t rawtime;
  time(&rawtime);
  return localtime(&rawtime);
}


//------------------------------------------------------------------------------
// The number of CPU seconds the timer has been running.
//------------------------------------------------------------------------------
double Timer::elapsed_cpu_time() const
{
  if (running)
    return (get_cpu_time() - startCpuTime + accumulatedCpuTime) / CLOCKS_PER_SEC;

  return accumulatedCpuTime / CLOCKS_PER_SEC;
}


//------------------------------------------------------------------------------
// Then number of seconds the timer has been running.
//------------------------------------------------------------------------------
double Timer::elapsed_wall_time() const
{
  if (running)
    return get_wall_time() - startWallTime + accumulatedWallTime;

  return accumulatedWallTime;
}


//------------------------------------------------------------------------------
// Returns the current CPU time.
//------------------------------------------------------------------------------
double Timer::get_cpu_time() const
{
  return clock();
}


//------------------------------------------------------------------------------
// Returns the current wall time.
//------------------------------------------------------------------------------
double Timer::get_wall_time() const
{
  struct timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec + time.tv_usec * 0.000001;
}


//------------------------------------------------------------------------------
// Stops the timer and sets the accumulated time to zero
//------------------------------------------------------------------------------
void Timer::reset()
{
  running = false;
  accumulatedCpuTime = 0;
  accumulatedWallTime = 0;
}


//------------------------------------------------------------------------------
// Sets the accumulated time to zero and starts the timer
//------------------------------------------------------------------------------
void Timer::restart()
{
  startCpuTime = get_cpu_time();
  startWallTime = get_wall_time();
  accumulatedCpuTime = 0;
  accumulatedWallTime = 0;
  running = true;
}


//------------------------------------------------------------------------------
// Starts the timer.
//------------------------------------------------------------------------------
void Timer::start()
{
  if (!running)
  {
    startCpuTime = get_cpu_time();
    startWallTime = get_wall_time();
    running = true;
  }
}


//------------------------------------------------------------------------------
// Stops the timer. Can continue by calling start()
//------------------------------------------------------------------------------
void Timer::stop()
{
  if (running)
  {
    accumulatedCpuTime += get_cpu_time() - startCpuTime;
    accumulatedWallTime += get_wall_time() - startWallTime;
    running = false;
  }
}

