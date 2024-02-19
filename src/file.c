#include "file.h"

#include <stdio.h>
#include <stdlib.h>

char *file_read_all(const char *path)
{
  FILE *fp = fopen(path, "rb");
  
  if (!fp) {
    perror(path);
    return NULL;
  }
  
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  
  char *buffer = malloc(fsize + 1);
  fread(buffer, fsize, 1, fp);
  fclose(fp);
  
  buffer[fsize] = 0;
  
  return buffer;
}
