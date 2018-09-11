#include <unistd.h>
#include <stdio.h>
#include <acc_timer.h>

int main(int argc, char *argv[]) {
  printf("begin\n");
  acc_timer_ctx_t *timer = initialize_acc_timer(1);
  acc_timer_begin(timer, "Test timing");
  sleep(1);
  acc_timer_end(timer, "Test timing");
  printf("end\n");
  acc_write(timer);
}
