#include <stdio.h>

#include "haruko.h"
#include "setup.h"

int main(int argc, char *argv[])
{
  const char *name = NULL;
  
  if (argc == 1) {
    name = "GLCL";
  } else if (argc == 2) {
    name = argv[1];
  } else {
    fprintf(stderr, "usage: %s [GLCL]\n", argv[0]);
    return 1;
  }
  
  if (!setup_load(name)) {
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

