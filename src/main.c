#include <stdio.h>

#include "haruko.h"
#include "setup.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "usage: %s [cfg]\n", argv[0]);
    return 1;
  }
  
  if (!setup_load(argv[1])) {
    return 1;
  }
  
  if (!haruko_init()) {
    return 1;
  }
  
  if (!setup_run()) {
    return 1;
  }
  
  while (!haruko_should_quit()) {
    haruko_update();
  }
  
  haruko_quit();
  
  return 0;
}

