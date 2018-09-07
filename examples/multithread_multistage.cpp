#include <unistd.h>
#include <acc_timer.h>
#include <iostream>
#include <fstream>
#include <pthread.h>

static acc_timer_ctx_t *acc_timer;

void *do_time_session(void *nada) {
  int i,j;

  acc_timer_begin(acc_timer, "test1");
  sleep(1);
  acc_timer_end(acc_timer, "test1");

  for(j = 0; j < 3; j++) {
    acc_timer_begin(acc_timer, "test2");
    sleep(1);
    acc_timer_end(acc_timer, "test2");
  }
}

int main(int argc, char *argv[]) {
  int nts = 3, i;

  acc_timer = initialize_acc_timer(nts);
  
  pthread_t threads[nts];
  for(i = 0; i < nts; i++) {
    if(pthread_create(&threads[i], NULL, do_time_session, NULL)) {
      perror("Thraed creation failed");
      exit(1);
    }
  }

  for(i = 0; i < nts; i++) {
    if(pthread_join(threads[i], NULL)) {
      perror("Error joining thread");
      exit(1);
    }
  }

  acc_write(acc_timer);
}
