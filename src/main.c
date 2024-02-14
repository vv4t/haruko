#include <stdio.h>

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "file.h"
#include "gl.h"
#include "quad.h"
#include "shader.h"
#include "shader_setup.h"

int main(int argc, char *argv[])
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
  
  SDL_Window *window = SDL_CreateWindow(
    "haruko",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    800,
    600,
    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
  );
  
  if (!window) {
    fprintf(stderr, "failed to create SDL window.\n");
    return -1;
  }
  
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  
  if (!gl_context) {
    fprintf(stderr, "Failed to create GL context\n");
    return -1;
  }
  
  if (!gl_init()) {
    return -1;
  }
  
  quad_init();
  
  GLuint shader;
  
  shader_setup_t shader_setup;
  shader_setup_init(&shader_setup, "shader");
  shader_setup_add(&shader_setup, SHADER_BOTH, "#version 300 es\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "precision mediump float;\n");
  
  if (!shader_setup_source(&shader_setup, SHADER_VERTEX, "builtin/shader.vert")) {
    return -1;
  }
  
  if (!shader_setup_source(&shader_setup, SHADER_FRAGMENT, "builtin/input.glsl")) {
    return -1;
  }
  
  if (!shader_setup_source(&shader_setup, SHADER_FRAGMENT, "shader/grid_3d.frag")) {
    return -1;
  }
  
  if (!shader_setup_source(&shader_setup, SHADER_FRAGMENT, "builtin/shader.frag")) {
    return -1;
  }
  
  if (!shader_setup_compile(&shader, &shader_setup)) {
    return -1;
  }
  
  GLuint iTime = glGetUniformLocation(shader, "iTime");
  
  bool quit = false;
  
  quad_bind();
  glUseProgram(shader);
  
  float time = 0.0;
  
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        quit = true;
        break;
      }
    }
    
    glUniform1f(iTime, time);
    quad_draw();
    
    SDL_GL_SwapWindow(window);
    
    time += 0.015;
  }
  
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}
