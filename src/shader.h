#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include "gl.h"

bool shader_load(GLuint *shader, const char *vert_src, const char *frag_src);

#endif
