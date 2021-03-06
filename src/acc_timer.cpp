#include <iostream>
#include <fstream>
#include <chrono>
#include <pthread.h>
#include <map>
#include <vector>
#include <iomanip>
#include <sstream>

#include <cstring>

// acc_timer headers
#include "acc_timer.h"
#include "sys_util.hpp"

using namespace std;
using namespace std::chrono;


/*
 * data structure definitions
 */
/*
 * Each thread will maintain its own begin/end and accumulation
 */
struct _thread_acc_timer_ctx {
  /*
   *    * Each thread maintains its own begin/end
   *       */
  std::map<char*, time_point<high_resolution_clock>> begin_times;

  /*
   *    * The accumulated times are global
   *       */
  std::map<char*, duration<double>> accumulated_times;

  /*
   *    * Maintains a time history of how long each duration took
   *       * for each identifier
   *          */
  std::map<char*,std::vector<duration<double>>> time_history;

};
/*
 * Accumulation of all threads individual begin/end
 */
struct _acc_timer_ctx {
  int n_threads;
  long map_idx_ctr; // gives each thread an id to access its index in the vector
  std::map<long,int> tid_to_idx_map; // maps system tid to internal vector index
  std::vector<thread_acc_timer_ctx_t*> thread_timers;

  /*
   *    * Internally hold the names of the timeable periods
   *       */
  std::vector<char*> names_internal;

};


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

pthread_mutex_t idx_mutex;

/*
 * Gets the thread_timers index for the current thread 
 */
int __get_acc_thread_idx(acc_timer_ctx_t *ctx) {
  long tid = acc_timer_gettid();
  int idx;

  // Get the thread index
  if(ctx->tid_to_idx_map.count(tid) == 0) {
    // Tid doesn't exist, add and get index for tid
    pthread_mutex_lock(&idx_mutex);

    idx = __sync_fetch_and_add(&ctx->map_idx_ctr, 1);
    ctx->tid_to_idx_map[tid] = idx;

    pthread_mutex_unlock(&idx_mutex);
  } else {
    // IDX exists
    idx = ctx->tid_to_idx_map[tid];
  }
  return idx;
}

/*
 * Gets the acc_timer_ctx for the current thread
 */
thread_acc_timer_ctx_t *__get_my_thread_timer(acc_timer_ctx_t *ctx) {
  int thread_idx = __get_acc_thread_idx(ctx);
  //printf("Thread idx is %d\n", thread_idx);
  thread_acc_timer_ctx_t *my_acc_timer = ctx->thread_timers.at(thread_idx);
  return my_acc_timer;
}

/**
 * Finds the internal name of a timeable region.
 * If region does not exist, this returns NULL
 *
 * @nullable
 */
char *get_timeable_region(acc_timer_ctx_t *ctx, char *name) {
  vector<char*>::iterator cit;
  for(cit = ctx->names_internal.begin(); cit != ctx->names_internal.end(); ++cit) {
    char *internal_name = *cit;
    if(strcmp(internal_name, name) == 0) {
      // Names are equivalent, we have a match
      return internal_name;
    }
  }
  return NULL;
}

/*
 * Add a timeable region if it does not already exist
 */
pthread_mutex_t names_mutex;
char *try_add_timeable_region(acc_timer_ctx_t *ctx, char *name) {

  char *internal_name = get_timeable_region(ctx, name);
  if(internal_name == NULL) {
    pthread_mutex_lock(&names_mutex);
    internal_name = get_timeable_region(ctx, name);
    if(internal_name == NULL) {

      // If we make it here, we have to add the name internally
      // TODO: Create destructor and free this
      char *internal_copy = (char*) malloc(sizeof(char) * (strlen(name) + 1)); 
      strcpy(internal_copy, name);
      ctx->names_internal.push_back(internal_copy);
      internal_name = internal_copy;
    }

    pthread_mutex_unlock(&names_mutex);
  }

  return internal_name;
}

void acc_timer_begin(acc_timer_ctx_t *ctx, char *id) {
  thread_acc_timer_ctx_t *my_acc_timer = __get_my_thread_timer(ctx);
  //map<char*,time_point<high_resolution_clock>> begin_times = my_acc_timer->begin_times;
  
  char *internal_id = try_add_timeable_region(ctx, id);

  my_acc_timer->begin_times[internal_id] = high_resolution_clock::now();
}

void acc_timer_end(acc_timer_ctx_t *ctx, char *id) {
  // Put this first here to make it closer to real
  auto end_time = high_resolution_clock::now(); 

  thread_acc_timer_ctx_t *my_acc_timer = __get_my_thread_timer(ctx);
  map<char*,time_point<high_resolution_clock>> begin_times = my_acc_timer->begin_times;
  map<char*,duration<double>> acc_times = my_acc_timer->accumulated_times;

  //TODO:  Probably implement a get_timeable_region
  id = get_timeable_region(ctx, id);

  if(id == NULL || begin_times.count(id) == 0) {
    /* 
     * Ending a key that doesn't exist..
     * TODO: Add an error message or something */
    cout << "End times doesn't have id " << id << endl;
    return;
  }

  auto diff = duration_cast<nanoseconds>(end_time - begin_times[id]);

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
    my_acc_timer->accumulated_times[id] = duration_cast<nanoseconds>(diff + acc_times[id]);
  }

}


/*
 * Write the accumulations
 */
void acc_write_stream(acc_timer_ctx_t *ctx, std::ostream& out) {
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
        out << "TID " << tid << endl;
        break;
      }
    }

   thread_acc_timer_ctx_t *thread_ctx = ctx->thread_timers.at(i);
   map<char*, duration<double>>::iterator amit;
   map<char*, duration<double>> accmap = thread_ctx->accumulated_times;
   
   for(amit = accmap.begin(); amit != accmap.end(); ++amit) {
      char* name = amit->first;
      duration<double> accdur = amit->second;
      long long total = duration_cast<nanoseconds>(accdur).count();
      long long average = total/thread_ctx->time_history[name].size();

      out << name << "(Total, Average: " << total << ", " << average << "): ";

      if(glbl_accs.count(name) == 0) {
        glbl_accs[name] = accdur;
      } else {
        glbl_accs[name] = accdur + glbl_accs[name];
      }

      vector<duration<double>> history = thread_ctx->time_history[name];
      vector<duration<double>>::iterator hit;
      for(hit = history.begin(); hit != history.end(); ++hit) {

        duration<double> val = *hit;
        out << " " << duration_cast<nanoseconds>(val).count();
      }
      out << endl;
   }
   out << endl;// break between threads
  }

  out << endl << "Global Accumulations: " << endl;
  map<char*,duration<double>>::iterator glit;
  for(glit = glbl_accs.begin(); glit != glbl_accs.end(); ++glit) {
    char *name = glit->first;
    duration<double> accdur = glit->second;
    out << name << ": " << duration_cast<nanoseconds>(accdur).count() << endl;
  }
}

/*
 * Get the seconds that have been spent in a region
 *
 */
double get_seconds_in_region(acc_timer_ctx_t *ctx, const char *region) {
  int i;

  double total = 0;
  for(i = 0; i < ctx->n_threads; i++) {

    thread_acc_timer_ctx_t *thread_ctx = ctx->thread_timers.at(i);
    map<char*, duration<double>>::iterator amit;
    map<char*, duration<double>> accmap = thread_ctx->accumulated_times;

    for(amit = accmap.begin(); amit != accmap.end(); ++amit) {
      char* name = amit->first;
      if(strcmp(name, region) == 0) {
        duration<double> accdur = amit->second;
        total += accdur.count();
      }
    }
  }

  return total;
}

/*
 * Write the accumulations without output stream
 */
static int filecount = 0; //increment this
void acc_write(acc_timer_ctx_t *ctx) {

  stringstream filename;
  filename << "timings" << std::setw(4) << std::setfill('0') << filecount++ << ".out";

  std::ofstream ofs(filename.str().c_str(), std::ofstream::out);
  acc_write_stream(ctx, ofs);
  ofs.close();
}
