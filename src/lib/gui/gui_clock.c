#include "gui.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

/* System clocks.
 * TODO These are for Linux. Not sure if there are other things we should use, for Windows and MacOS.
 */
 
double gui_now_real() {
  struct timespec tv={0};
  clock_gettime(CLOCK_REALTIME,&tv);
  return (double)tv.tv_sec+(double)tv.tv_nsec/1000000000.0;
}

double gui_now_cpu() {
  // If CPU time is not available, use real time instead. Do not report zeroes, it must be something that increases over time.
  struct timespec tv={0};
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&tv);
  return (double)tv.tv_sec+(double)tv.tv_nsec/1000000000.0;
}

void gui_sleep(double s) {
  int us=(int)(s*1000000.0);
  if (us<0) us=0;
  usleep(us);
}

/* Initialize clock.
 */

void gui_clock_init(struct gui_clock *clock,double rate_hz) {
  if ((rate_hz<1.0)||(rate_hz>1000.0)) rate_hz=60.0;
  clock->period=1.0/rate_hz;
  clock->starttime_real=gui_now_real();
  clock->starttime_cpu=gui_now_cpu();
  clock->prevtime=clock->nexttime=clock->starttime_real;
  clock->framec=0;
  clock->panicc=0;
  
  // Now lie a little bit about (prevtime), pretend it's one cycle ago.
  // The first tick won't sleep, but we want it to report a sane interval.
  clock->prevtime-=clock->period;
}

/* Tick.
 */

double gui_clock_tick(struct gui_clock *clock) {

  double now=gui_now_real();
  while (now<clock->nexttime) {
    double sleeptime=clock->nexttime-now+0.001; // Plus a millisecond in case the system sleeps a little short, eg rounding error.
    // Negative sleep time, or substantially slower than 1 Hz, means the clock is broken.
    // Log a panic, reset, and don't sleep.
    if ((sleeptime<0.0)||(sleeptime>2.0)) {
      clock->panicc++;
      clock->nexttime=now;
      clock->prevtime=now-clock->period;
      break;
    }
    gui_sleep(sleeptime);
    now=gui_now_real();
  }
  
  double elapsed=now-clock->prevtime;
  clock->nexttime+=clock->period;
  clock->framec++;
  return elapsed;
}

/* Report.
 */

void gui_clock_report(struct gui_clock *clock) {
  if (clock->framec<1) return; // Insufficient data, do nothing.
  double now_real=gui_now_real();
  double now_cpu=gui_now_cpu();
  double elapsed_real=now_real-clock->starttime_real;
  double elapsed_cpu=now_cpu-clock->starttime_cpu;
  if (elapsed_real<=0.0) return; // Clock broken or something, do nothing.
  double avgrate=(double)clock->framec/elapsed_real;
  double cpuload=elapsed_cpu/elapsed_real;
  fprintf(stderr,
    "%d frames in %.03f s. %d clock panics. Average rate %.03f Hz. CPU load %.06f.\n",
    clock->framec,elapsed_real,clock->panicc,avgrate,cpuload
  );
}
