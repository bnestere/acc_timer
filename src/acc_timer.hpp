#ifndef __ACC_TIMER__
#define __ACC_TIMER__

#include <iostream>
#include <map>
#include <vector>

#include <chrono>

using namespace std;
using namespace std::chrono;

/*
 * Each thread will maintain its own begin/end and accumulation
 */
typedef struct _thread_acc_timer_ctx {
  /*
   * Each thread maintains its own begin/end
   */
  std::map<char*, time_point<high_resolution_clock>> begin_times; 

  /*
   * The accumulated times are global
   */
  std::map<char*, duration<double>> accumulated_times;

  /*
   * Maintains a time history of how long each duration took
   * for each identifier
   */
  std::map<char*,std::vector<duration<double>>> time_history;

} thread_acc_timer_ctx_t;


/*
 * Accumulation of all threads individual begin/end
 */
typedef struct _acc_timer_ctx {
  int n_threads;
  long map_idx_ctr; // gives each thread an id to access its index in the vector
  std::map<long,int> tid_to_idx_map; // maps system tid to internal vector index
  std::vector<thread_acc_timer_ctx_t*> thread_timers;
} acc_timer_ctx_t;

/*
 * Starts timing for an id
 *
 * Arguments
 *  timer_ctx: the timer context
 *  id: The unique name of the section to time
 */
void acc_timer_begin(acc_timer_ctx_t *timer_ctx, char *id);

/*
 * Ends timing for an id and add it to the accumulations
 *
 * Arguments
 *  timer_ctx: the timer context
 *  id: The unique name of the section to time
 */
void acc_timer_end(acc_timer_ctx_t *timer_ctx, char *id);

/*
 * Writes the timer accumulations 
 *
 * Arguments
 *  timer_ctx: the timer context
 *  out: the output to write accumulated times to
 */
void acc_write(acc_timer_ctx_t *timer_ctx, std::ostream& out);

/*
 * Writes the timer accumulations 
 *
 * Arguments
 *  timer_ctx: the timer context
 */
void acc_write(acc_timer_ctx_t *timer_ctx);

acc_timer_ctx_t *initialize_acc_timer(int n_threads);
void destroy_acc_timer(acc_timer_ctx_t *timer_ctx);

#endif
