#include "buffer.h"

#include "quad.h"
#include "shader.h"
#include "shader_setup.h"

buffer_t buffer_main(channel_t empty)
{
  buffer_t buffer = {
    .fbo = 0,
    .texture = 0,
    .channel = {
      empty,
      empty,
      empty,
      empty
    }
  };
  
  return buffer;
}

buffer_t buffer_new(channel_t empty, int width, int height)
{
  buffer_t buffer;
  
  glGenTextures(1, &buffer.texture);
  glBindTexture(GL_TEXTURE_2D, buffer.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  glGenFramebuffers(1, &buffer.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.texture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  buffer.channel[0] = empty;
  buffer.channel[1] = empty;
  buffer.channel[2] = empty;
  buffer.channel[3] = empty;
  
  return buffer;
}

void buffer_update(buffer_t *buffer)
{
  glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
  
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(buffer->shader);
  
  for (int i = 0; i < 4; i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(buffer->channel[i].type, buffer->channel[i].texture);
  }
  
  quad_draw();
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool buffer_shader_load(buffer_t *buffer, const char *shader_path)
{
  shader_setup_t shader_setup;
  shader_setup_init(&shader_setup, "shader");
  shader_setup_add(&shader_setup, SHADER_BOTH, "#version 300 es\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "precision mediump float;\n");
  
  shader_setup_add(&shader_setup, SHADER_BOTH, "layout (std140) uniform ub_input {\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "  vec4 iMouse;\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "  vec3 iResolution;\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "  float iTime;\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "};\n");
  
  for (int i = 0; i < 4; i++) {
    char channel_name[32];
    
    switch (buffer->channel[i].type) {
    case GL_TEXTURE_2D:
      snprintf(channel_name, 32, "uniform sampler2D iChannel%i;\n", i);
      shader_setup_add(&shader_setup, SHADER_FRAGMENT, channel_name);
      break;
    case GL_TEXTURE_CUBE_MAP:
      snprintf(channel_name, 32, "uniform samplerCube iChannel%i;\n", i);
      shader_setup_add(&shader_setup, SHADER_FRAGMENT, channel_name);
      break;
    default:
      break;
    }
  }
  
  if (!shader_setup_source(&shader_setup, SHADER_VERTEX, "builtin/shader.vert")) return false;
  if (!shader_setup_source(&shader_setup, SHADER_FRAGMENT, shader_path)) return false;
  if (!shader_setup_source(&shader_setup, SHADER_FRAGMENT, "builtin/shader.frag")) return false;
  if (!shader_setup_compile(&buffer->shader, &shader_setup)) return false;
  
  shader_setup_free(&shader_setup);
  
  glUseProgram(buffer->shader);
  
  for (int i = 0; i < 4; i++) {
    char channel_name[32];
    snprintf(channel_name, 32, "iChannel%i", i);
    
    GLuint ul_channel = glGetUniformLocation(buffer->shader, channel_name);
    glUniform1i(ul_channel, i);
  }
  
  GLuint ubl_input = glGetUniformBlockIndex(buffer->shader, "ub_input");
  glUniformBlockBinding(buffer->shader, ubl_input, 0);
  
  return true;
}

void channel_init(channel_t *channel, GLuint type, GLuint texture)
{
  channel->type = type;
  channel->texture = texture;
}
