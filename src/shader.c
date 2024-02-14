#include "shader.h"

#include <stdio.h>
#include "gl.h"

#define MAX_INFO 512

static bool shader_compile(GLuint *shader, const char *src[], int num_src, GLuint type);

bool shader_load(GLuint *shader, const char *vert_src[], int num_vert_src, const char *frag_src[], int num_frag_src)
{
  *shader = glCreateProgram();
  
  GLuint vert_shader;
  if (!shader_compile(&vert_shader, vert_src, num_vert_src, GL_VERTEX_SHADER)) {
    return false;
  }
  
  GLuint frag_shader;
  if (!shader_compile(&frag_shader, frag_src, num_frag_src, GL_FRAGMENT_SHADER)) {
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
    fprintf(stderr, "Failed to link shader:\n%s\n", info);
    return false;
  }
  
  glDetachShader(*shader, vert_shader);
  glDeleteShader(vert_shader);

  glDetachShader(*shader, frag_shader);
  glDeleteShader(frag_shader);
  
  return true;
}

static bool shader_compile(GLuint *shader, const char *src[], int num_src, GLuint type)
{
  int success;
  static GLchar info[MAX_INFO];
  
  *shader = glCreateShader(type);
  glShaderSource(*shader, num_src, src, NULL);
  
  glCompileShader(*shader);
  glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
  
  if (!success) {
    const char *name;
    
    switch (type) {
    case GL_VERTEX_SHADER:
      name = "VERTEX_SHADER";
      break;
    case GL_FRAGMENT_SHADER:
      name = "FRAGMENT_SHADER";
      break;
    default:
      name = "UNKNOWN";
      break;
    }
    
    glGetShaderInfoLog(*shader, MAX_INFO, NULL, info);
    fprintf(stderr, "Failed to compiler %s:\n%s\n", name, info);
    
    return false;
  }
  
  return true;
}
