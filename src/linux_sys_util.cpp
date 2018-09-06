#include <cstdlib>
#include <unistd.h>
#include <sys/syscall.h>

// acc_timer includes
#include "sys_util.hpp"

long acc_timer_gettid(void) {
  return syscall(__NR_gettid);
}
