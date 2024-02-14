#include <stdio.h>

#define HARUKO_WIDTH 640
#define HARUKO_HEIGHT 360

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "file.h"
#include "gl.h"
#include "quad.h"
#include "shader.h"
#include "shader_setup.h"

typedef struct {
  GLuint fbo;
  GLuint texture;
} frame_t;

struct {
  SDL_Window *window;
  SDL_GLContext gl_context;
  
  bool quit;
  
  float mouse_x;
  float mouse_y;
  float mouse_down;
  
  GLuint ul_iTime;
  GLuint ul_iMouse;
} haruko;

void haruko_poll();
void haruko_quit();
bool haruko_load_sdl(const char *title, int width, int height);
bool haruko_load_shader(GLuint *shader, const char *path);
bool haruko_load_image(GLuint *texture, const char *path);

frame_t frame_new(int width, int height);
void frame_begin(frame_t frame);
void frame_end();

int main(int argc, char *argv[])
{
  if (!haruko_load_sdl("haruko", HARUKO_WIDTH, HARUKO_HEIGHT)) {
    return false;
  }
  
  if (!gl_init()) {
    return -1;
  }
  
  quad_init();
  
  GLuint shader;
  if (!haruko_load_shader(&shader, "shader/image_test/image.glsl")) {
    return false;
  }
  
  GLuint iChannel0;
  
  if (!haruko_load_image(&iChannel0, "shader/image_test/cirno.png")) {
    return false;
  }
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, iChannel0);
  
  quad_bind();
  
  float time = 0.0;
  
  while (!haruko.quit) {
    haruko_poll();
    
    glUseProgram(shader);
    glUniform1f(haruko.ul_iTime, time);
    glUniform3f(haruko.ul_iMouse, haruko.mouse_x, haruko.mouse_y, haruko.mouse_down);
    quad_draw();
    
    time += 0.015;
    
    SDL_GL_SwapWindow(haruko.window);
  }
  
  haruko_quit();
  
  return 0;
}

void shader_frame_test()
{
  /*
  frame_t frame = frame_new(640, 360);
  
  GLuint iChannel0 = frame.texture;
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, iChannel0);
  
  GLuint shader;
  if (!haruko_load_shader(&shader, "shader/frame_test/image.glsl")) {
    return false;
  }
  
  GLuint shader2;
  if (!haruko_load_shader(&shader2, "shader/frame_test/iChannel0.glsl")) {
    return false;
  }
  
  GLuint ul_iChannel0 = glGetUniformLocation(shader, "iChannel0");
  
  glUniform1i(ul_iChannel0, 0);
  
  --------------------------------------
  
  frame_begin(frame);
  glUseProgram(shader2);
  quad_draw();
  frame_end();
  
  glUseProgram(shader);
  glUniform1f(ul_iTime, time);
  quad_draw();
  
  return true;
  */
}

frame_t frame_new(int width, int height)
{
  frame_t frame;
  
  glGenTextures(1, &frame.texture);
  glBindTexture(GL_TEXTURE_2D, frame.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  glGenFramebuffers(1, &frame.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, frame.fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame.texture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  return frame;
}

void frame_begin(frame_t frame)
{
  glBindFramebuffer(GL_FRAMEBUFFER, frame.fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void frame_end()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint bitmap_format(SDL_Surface *bitmap);

bool haruko_load_image(GLuint *texture, const char *path)
{
  SDL_Surface *bitmap = IMG_Load(path);
  
  if (!bitmap) {
    fprintf(stderr, "could not load image '%s'", path);
    return false;
  }
  
  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_2D, *texture);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  GLuint internal_format = bitmap_format(bitmap);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->w, bitmap->h, 0, internal_format, GL_UNSIGNED_BYTE, bitmap->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  SDL_FreeSurface(bitmap);
  
  return true;
}

GLuint bitmap_format(SDL_Surface *bitmap)
{
  switch (bitmap->format->BytesPerPixel) {
  case 3:
    return GL_RGB;
  case 4:
    return GL_RGBA;
  default:
    fprintf(stderr, "unknown format: %i\n", bitmap->format->BytesPerPixel);
    return 0;
  }
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
    case SDL_MOUSEBUTTONDOWN:
      haruko.mouse_down = 1.0;
      break;
    case SDL_MOUSEBUTTONUP:
      haruko.mouse_down = 0.0;
      break;
    case SDL_MOUSEMOTION:
      haruko.mouse_x = event.motion.x;
      haruko.mouse_y = HARUKO_HEIGHT - event.motion.y;
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
  
  shader_setup_add(&shader_setup, SHADER_BOTH, "uniform float iTime;\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "uniform vec2 iResolution;\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "uniform vec3 iMouse;\n");
  shader_setup_add(&shader_setup, SHADER_FRAGMENT, "uniform sampler2D iChannel0;\n");
  
  if (!shader_setup_source(&shader_setup, SHADER_VERTEX, "builtin/shader.vert")) {
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
  
  glUseProgram(*shader);
  
  GLuint ul_iResolution = glGetUniformLocation(*shader, "iResolution");
  
  glUniform2f(ul_iResolution, HARUKO_WIDTH, HARUKO_HEIGHT);
  
  haruko.ul_iTime = glGetUniformLocation(*shader, "iTime");
  haruko.ul_iMouse = glGetUniformLocation(*shader, "iMouse");
  
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
