#include <iostream>
#include <chrono>

// acc_timer headers
#include "acc_timer.hpp"
#include "sys_util.hpp"

using namespace std;
using namespace std::chrono;

acc_timer_ctx_t *initialize_acc_timer(int n_threads) {
  acc_timer_ctx_t *ctx = new acc_timer_ctx_t();
  ctx->n_threads = n_threads;
  ctx->map_idx_ctr = 0; // start with index 0
  ctx->thread_timers.reserve(n_threads);

  int i;
  for(i = 0; i < n_threads; i++) {
    ctx->thread_timers.push_back(new thread_acc_timer_ctx_t());
  }

  return ctx;
}

/*
 * Gets the thread_timers index for the current thread 
 */
int __get_acc_thread_idx(acc_timer_ctx_t *ctx) {
  long tid = acc_timer_gettid();
  int idx;

  // Get the thread index
  if(ctx->tid_to_idx_map.count(tid) == 0) {
    // Tid doesn't exist, add and get index for tid
    idx = __sync_fetch_and_add(&ctx->map_idx_ctr, 1);
    ctx->tid_to_idx_map.insert(pair<long,int>(tid, idx));
  } else {
    idx = ctx->tid_to_idx_map[tid];
  }
  return idx;
}

/*
 * Gets the acc_timer_ctx for the current thread
 */
thread_acc_timer_ctx_t *__get_my_thread_timer(acc_timer_ctx_t *ctx) {
  int thread_idx = __get_acc_thread_idx(ctx);
  thread_acc_timer_ctx_t *my_acc_timer = ctx->thread_timers.at(thread_idx);
  return my_acc_timer;
}

void acc_timer_begin(acc_timer_ctx_t *ctx, char *id) {
  thread_acc_timer_ctx_t *my_acc_timer = __get_my_thread_timer(ctx);
  //map<char*,time_point<high_resolution_clock>> begin_times = my_acc_timer->begin_times;

  my_acc_timer->begin_times[id] = high_resolution_clock::now();
}

void acc_timer_end(acc_timer_ctx_t *ctx, char *id) {
  // Put this first here to make it closer to real
  auto end_time = high_resolution_clock::now(); 

  thread_acc_timer_ctx_t *my_acc_timer = __get_my_thread_timer(ctx);
  map<char*,time_point<high_resolution_clock>> begin_times = my_acc_timer->begin_times;
  map<char*,duration<double>> acc_times = my_acc_timer->accumulated_times;

  if(begin_times.count(id) == 0) {
    /* 
     * Ending a key that doesn't exist..
     * TODO: Add an error message or something */
    cout << "Begin times doesn't have id " << id << endl;
    return;
  }

  auto diff = duration_cast<microseconds>(end_time - begin_times[id]);

  // Save this period into history
  if(my_acc_timer->time_history.count(id) == 0) {
    my_acc_timer->time_history[id] = vector<duration<double>>();
  }
  my_acc_timer->time_history[id].push_back(diff);

  if(acc_times.count(id) == 0) {
    // Initialize with time

    my_acc_timer->accumulated_times[id] = diff;
  } else {
    // Add time
    my_acc_timer->accumulated_times[id] = duration_cast<microseconds>(diff + acc_times[id]);
  }

}

/*
 * Write the accumulations
 */
void acc_write(acc_timer_ctx_t *ctx, std::ostream& out) {
  int i;
  map<long,int> tidmap = ctx->tid_to_idx_map;

  map<char*, duration<double>> glbl_accs;

  for(i = 0; i < ctx->n_threads; i++) {
    
    //Print the individual tid we are looking at
    map<long,int>::iterator tidit;
    for(tidit = tidmap.begin(); tidit != tidmap.end(); ++tidit) {
      long tid = tidit->first;
      int idx_cmp = tidit->second;
      if(i == idx_cmp) {
        // This is the TID we are looking for
        out << "Index, " << i << ", TID, " << tid << endl;
        break;
      }
    }

   thread_acc_timer_ctx_t *thread_ctx = ctx->thread_timers.at(i);
   map<char*, duration<double>>::iterator amit;
   map<char*, duration<double>> accmap = thread_ctx->accumulated_times;
   
   for(amit = accmap.begin(); amit != accmap.end(); ++amit) {
      char* name = amit->first;
      duration<double> accdur = amit->second;
      out << name << "(" << duration_cast<microseconds>(accdur).count() << "):";

      if(glbl_accs.count(name) == 0) {
        glbl_accs[name] = accdur;
      } else {
        glbl_accs[name] = accdur + glbl_accs[name];
      }

      vector<duration<double>> history = thread_ctx->time_history[name];
      vector<duration<double>>::iterator hit;
      for(hit = history.begin(); hit != history.end(); ++hit) {

        duration<double> val = *hit;
        out << " " << duration_cast<microseconds>(val).count();
      }
      out << endl;
   }
  }

  out << endl << "Global Accumulations: " << endl;
  map<char*,duration<double>>::iterator glit;
  for(glit = glbl_accs.begin(); glit != glbl_accs.end(); ++glit) {
    char *name = glit->first;
    duration<double> accdur = glit->second;
    out << name << ": " << duration_cast<microseconds>(accdur).count() << endl;
  }
}
