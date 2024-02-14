#ifndef SHADER_SETUP_H
#define SHADER_SETUP_H

#include "gl.h"
#include <stdbool.h>

#define MAX_SRC 32

typedef struct {
  const char *name;
  char *vert_src[MAX_SRC];
  char *frag_src[MAX_SRC];
  int num_vert;
  int num_frag;
} shader_setup_t;

typedef enum {
  SHADER_VERTEX = 1,
  SHADER_FRAGMENT = 2,
  SHADER_BOTH = SHADER_VERTEX | SHADER_FRAGMENT
} shader_bit_t;

void shader_setup_init(shader_setup_t *shader_setup, const char *name);
void shader_setup_add(shader_setup_t *shader_setup, shader_bit_t shader, const char *text);
bool shader_setup_source(shader_setup_t *shader_setup, shader_bit_t shader, const char *path);
bool shader_setup_compile(GLuint *shader, const shader_setup_t *shader_setup);
void shader_setup_free(shader_setup_t *shader_setup);

#endif
