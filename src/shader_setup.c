#include "shader_setup.h"

#include <string.h>
#include <stdlib.h>
#include "file.h"
#include "shader.h"

void shader_setup_init(shader_setup_t *shader_setup, const char *name)
{
  shader_setup->num_vert = 0;
  shader_setup->num_frag = 0;
  shader_setup->name = name;
}

void shader_setup_add(shader_setup_t *shader_setup, shader_bit_t shader, const char *text)
{
  if (shader & SHADER_VERTEX) {
    if (shader_setup->num_vert == MAX_SRC) {
      printf("warning: too many vertex sources\n");
    } else {
      shader_setup->vert_src[shader_setup->num_vert] = strdup(text);
      shader_setup->num_vert += 1;
    }
  }
  
  if (shader & SHADER_FRAGMENT) {
    if (shader_setup->num_frag == MAX_SRC) {
      printf("warning: too many fragment sources\n");
    } else {
      shader_setup->frag_src[shader_setup->num_frag] = strdup(text);
      shader_setup->num_frag += 1;
    }
  }
}

bool shader_setup_source(shader_setup_t *shader_setup, shader_bit_t shader, const char *path)
{
  char *text = file_read_all(path);
  
  if (!text) {
    fprintf(stderr, "failed to load '%s'\n", path);
    return false;
  }
  
  shader_setup_add(shader_setup, shader, text);
  
  free(text);
  
  return true;
}

bool shader_setup_compile(GLuint *shader, const shader_setup_t *shader_setup)
{
  if (
    !shader_load(
      shader,
      (const char **) shader_setup->vert_src, shader_setup->num_vert,
      (const char **) shader_setup->frag_src, shader_setup->num_frag
    )
  ) {
    fprintf(stderr, "failed to compile shader setup: '%s'\n", shader_setup->name);
    return false;
  }
  
  return true;
}

void shader_setup_free(shader_setup_t *shader_setup)
{
  for (int i = 0; i < shader_setup->num_vert; i++) {
    free(shader_setup->vert_src[i]);
    shader_setup->vert_src[i] = NULL;
  }
  
  for (int i = 0; i < shader_setup->num_frag; i++) {
    free(shader_setup->frag_src[i]);
    shader_setup->frag_src[i] = NULL;
  }
  
  shader_setup->num_vert = 0;
  shader_setup->num_frag = 0;
}
