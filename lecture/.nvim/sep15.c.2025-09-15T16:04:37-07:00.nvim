#include <stdio.h>
#include <unistd.h>

int main() {
  for (int i = 1; i < 20; i++) {
    int pid = getpid();
    fork();
    int pid2 = getpid();
    printf("sleeping %d\n", i);
    sleep(1);
  }
  printf("done");
}
