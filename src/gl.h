#ifndef GL_H
#define GL_H

#include <stdio.h>
#include <stdbool.h>
#include <GL/glew.h>

inline static bool gl_init()
{
  glewExperimental = true;
  
  GLenum status = glewInit();
  
  if (status != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW: %s", glewGetErrorString(status));
    return false;
  }
  
  return true;
}

#endif
