#include <unistd.h>
#include <stdio.h>
#include <acc_timer.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  printf("begin\n");
  acc_timer_ctx_t *timer = initialize_acc_timer(1);
  acc_timer_begin(timer, "Test timing");
  usleep(500000);
  acc_timer_end(timer, "Test timing");
  printf("end\n");
  acc_write(timer);

  double seconds = get_seconds_in_region(timer, "Test timing");
  printf("Total time in region: %.8f seconds\n", seconds);
}
