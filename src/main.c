#include <stdio.h>

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "file.h"
#include "gl.h"
#include "quad.h"
#include "shader.h"
#include "shader_setup.h"

struct {
  SDL_Window *window;
  SDL_GLContext gl_context;
  bool quit;
} haruko;

void haruko_poll();
void haruko_quit();
bool haruko_load_sdl(const char *title, int width, int height);
bool haruko_load_shader(GLuint *shader, const char *path);

int main(int argc, char *argv[])
{
  if (!haruko_load_sdl("haruko", 640, 360)) {
    return false;
  }
  
  if (!gl_init()) {
    return -1;
  }
  
  quad_init();
  
  GLuint shader;
  
  if (!haruko_load_shader(&shader, "shader/grid_3d.glsl")) {
    return false;
  }
  
  GLuint iTime = glGetUniformLocation(shader, "iTime");
  
  quad_bind();
  glUseProgram(shader);
  
  float time = 0.0;
  
  while (!haruko.quit) {
    haruko_poll();
    
    glUniform1f(iTime, time);
    quad_draw();
    
    time += 0.015;
    
    SDL_GL_SwapWindow(haruko.window);
  }
  
  haruko_quit();
  
  return 0;
}

void haruko_quit()
{
  SDL_GL_DeleteContext(haruko.gl_context);
  SDL_DestroyWindow(haruko.window);
  SDL_Quit();
}

void haruko_poll()
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      haruko.quit = true;
      break;
    }
  }
}

bool haruko_load_shader(GLuint *shader, const char *path)
{
  shader_setup_t shader_setup;
  shader_setup_init(&shader_setup, "shader");
  shader_setup_add(&shader_setup, SHADER_BOTH, "#version 300 es\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "precision mediump float;\n");
  
  if (!shader_setup_source(&shader_setup, SHADER_VERTEX, "builtin/shader.vert")) {
    return false;
  }
  
  if (!shader_setup_source(&shader_setup, SHADER_FRAGMENT, "builtin/input.glsl")) {
    return false;
  }
  
  if (!shader_setup_source(&shader_setup, SHADER_FRAGMENT, path)) {
    return false;
  }
  
  if (!shader_setup_source(&shader_setup, SHADER_FRAGMENT, "builtin/shader.frag")) {
    return false;
  }
  
  if (!shader_setup_compile(shader, &shader_setup)) {
    return false;
  }
  
  shader_setup_free(&shader_setup);
  
  return true;
}

bool haruko_load_sdl(const char *title, int width, int height)
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  
  haruko.window = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    width, height,
    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
  );
  
  if (!haruko.window) {
    fprintf(stderr, "failed to create SDL window.\n");
    return true;
  }
  
  haruko.gl_context = SDL_GL_CreateContext(haruko.window);
  
  if (!haruko.gl_context) {
    fprintf(stderr, "Failed to create GL context\n");
    return false;
  }
  
  return true;
}
