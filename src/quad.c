#include "quad.h"

#include "gl.h"

typedef struct {
  float x;
  float y;
  float u;
  float v;
} vertex_t;

struct { GLuint vbo; } quad;

void quad_init()
{
  vertex_t vertices[] = {
    { .x = -1.0f, .y = -1.0f, .u = 0.0f, .v = 0.0f },
    { .x = -1.0f, .y = +1.0f, .u = 0.0f, .v = 1.0f },
    { .x = +1.0f, .y = -1.0f, .u = 1.0f, .v = 0.0f },
    { .x = -1.0f, .y = +1.0f, .u = 0.0f, .v = 1.0f },
    { .x = +1.0f, .y = +1.0f, .u = 1.0f, .v = 1.0f },
    { .x = +1.0f, .y = -1.0f, .u = 1.0f, .v = 0.0f }
  };
  
  glGenBuffers(1, &quad.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
  glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(vertex_t), vertices, GL_STATIC_DRAW);
  
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (float*) 0);
  
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (float*) 0 + 2);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void quad_bind()
{
  glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
}

void quad_draw()
{
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
