#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include "gl.h"

bool shader_load(GLuint *shader, const char *vert_src[], int num_vert_src, const char *frag_src[], int num_frag_src);

#endif
