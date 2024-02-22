#include "shader_setup.h"

#include <string.h>
#include <stdlib.h>
#include "file.h"

#define MAX_INFO 512

static bool shader_compile(GLuint *shader, const char *name, const char *src[], int num_src, GLuint type);

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
    return false;
  }
  
  shader_setup_add(shader_setup, shader, text);
  
  free(text);
  
  return true;
}

bool shader_setup_compile(GLuint *shader, const shader_setup_t *shader_setup)
{
  *shader = glCreateProgram();
  
  const char *name = shader_setup->name;
  const char **vert_src = (const char **) shader_setup->vert_src;
  const char **frag_src = (const char **) shader_setup->frag_src;
  int num_vert_src = shader_setup->num_vert;
  int num_frag_src = shader_setup->num_frag;
  
  GLuint vert_shader;
  if (!shader_compile(&vert_shader, name, vert_src, num_vert_src, GL_VERTEX_SHADER)) {
    return false;
  }
  
  GLuint frag_shader;
  if (!shader_compile(&frag_shader, name, frag_src, num_frag_src, GL_FRAGMENT_SHADER)) {
    return false;
  }
  
  glAttachShader(*shader, vert_shader);
  glAttachShader(*shader, frag_shader);
  
  int success;
  static char info[MAX_INFO];
  
  glLinkProgram(*shader);
  glGetProgramiv(*shader, GL_LINK_STATUS, &success);
  
  if (!success) {
    glGetProgramInfoLog(*shader, MAX_INFO, NULL, info);
    fprintf(stderr, "%s: failed to link:\n%s\n", name, info);
    return false;
  }
  
  glDetachShader(*shader, vert_shader);
  glDeleteShader(vert_shader);

  glDetachShader(*shader, frag_shader);
  glDeleteShader(frag_shader);
  
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

static bool shader_compile(GLuint *shader, const char *name, const char *src[], int num_src, GLuint type)
{
  int success;
  static GLchar info[MAX_INFO];
  
  *shader = glCreateShader(type);
  glShaderSource(*shader, num_src, src, NULL);
  
  glCompileShader(*shader);
  glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
  
  if (!success) {
    const char *shader_name;
    
    switch (type) {
    case GL_VERTEX_SHADER:
      shader_name = "vertex";
      break;
    case GL_FRAGMENT_SHADER:
      shader_name = "fragment";
      break;
    default:
      shader_name = "unknown";
      break;
    }
    
    glGetShaderInfoLog(*shader, MAX_INFO, NULL, info);
    fprintf(stderr, "shader: %s: %s: failed to compile\n%s\n", name, shader_name, info);
    
    return false;
  }
  
  return true;
}
