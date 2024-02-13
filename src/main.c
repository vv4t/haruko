#include <stdio.h>

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "file.h"
#include "gl.h"
#include "quad.h"
#include "shader.h"

int main(int argc, char *argv[]) {
  
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
    fprintf(stderr, "Failed to create SDL window.\n");
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
  
  char *vert_src = file_read_all("assets/shader.vert");
  
  if (!vert_src) {
    fprintf(stderr, "Failed to load 'assets/shader.vert'\n");
    return -1;
  }
  
  char *frag_src = file_read_all("assets/shader.frag");
  
  if (!frag_src) {
    fprintf(stderr, "Failed to load 'assets/shader.frag'\n");
    return -1;
  }
  
  GLuint shader;
  
  if (!shader_load(&shader, vert_src, frag_src)) {
    fprintf(stderr, "Failed to load shader\n");
    return -1;
  }
  
  bool quit = false;
  
  quad_bind();
  glUseProgram(shader);
  
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        quit = true;
        break;
      }
    }
    
    quad_draw();
    
    SDL_GL_SwapWindow(window);
  }
  
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}
