#ifndef BUFFER_H
#define BUFFER_H

#include "gl.h"

typedef struct {
  GLuint type;
  GLuint texture;
} channel_t;

typedef struct {
  GLuint fbo;
  GLuint texture;
  GLuint shader;
  channel_t channel[4];
} buffer_t;

buffer_t buffer_main(channel_t empty);
buffer_t buffer_new(channel_t empty, int width, int height);
bool buffer_shader_load(buffer_t *buffer, const char *shader_path);
void buffer_update(buffer_t *buffer);

void channel_init(channel_t *channel, GLuint type, GLuint texture);

#endif
