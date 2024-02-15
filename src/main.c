#include <stdio.h>

#define HARUKO_WIDTH 640
#define HARUKO_HEIGHT 360

#define MAX_ICHANNEL 8

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "file.h"
#include "gl.h"
#include "quad.h"
#include "shader.h"
#include "shader_setup.h"

typedef enum {
  FRAME_NONE,
  FRAME_IMAGE,
  FRAME_SHADER,
  FRAME_CUBEMAP
} type_frame_t;

typedef struct {
  float iMouse_x;
  float iMouse_y;
  float iMouse_z;
  float iMouse_w;
  
  float iResolution_x;
  float iResolution_y;
  
  float iTime;
} ub_input_t;

typedef struct {
  type_frame_t type;
  
  const char *shader_path;
  const char *image_path;
  
  GLuint texture;
  
  GLuint fbo;
  GLuint shader;
  
  int width;
  int height;
} frame_t;

struct {
  SDL_Window *window;
  SDL_GLContext gl_context;
  
  bool quit;
  
  float mouse_x;
  float mouse_y;
  float mouse_down;
  
  GLuint ubo_input;
} haruko;

void haruko_poll();
void haruko_quit();
bool haruko_load_sdl(const char *title, int width, int height);
bool haruko_load_shader(GLuint *shader, frame_t frame[], const char *path);
bool haruko_load_image(GLuint *texture, const char *path);
void haruko_init_input();

frame_t frame_shader(const char *shader_path);
frame_t frame_image(const char *image_path);

bool frame_init(frame_t frame[], GLuint shader);
bool frame_image_load(frame_t frame[], int num, GLuint shader);
bool frame_shader_load(frame_t frame[], int num, GLuint shader);
void frame_update(frame_t frame[]);
void frame_shader_setup(GLuint shader, frame_t frame[]);

int main(int argc, char *argv[])
{
  if (!haruko_load_sdl("haruko", HARUKO_WIDTH, HARUKO_HEIGHT)) {
    return false;
  }
  
  if (!gl_init()) {
    return -1;
  }
  
  quad_init();
  haruko_init_input();
  
  frame_t iChannel[MAX_ICHANNEL] = {
    frame_shader("shader/frame_test/iChannel0.glsl"),
    frame_shader("shader/frame_test/iChannel1.glsl"),
    frame_image("shader/frame_test/boundaries.png")
  };
  
  GLuint image_shader;
  if (!haruko_load_shader(&image_shader, iChannel, "shader/frame_test/image.glsl")) {
    return false;
  }
  
  if (!frame_init(iChannel, image_shader)) {
    return false;
  }
  
  frame_shader_setup(image_shader, iChannel);
  
  quad_bind();
  
  float time = 0.0;
  
  while (!haruko.quit) {
    haruko_poll();
    
    ub_input_t ub_input = {
      .iMouse_x = haruko.mouse_x,
      .iMouse_y = haruko.mouse_y,
      .iMouse_z = haruko.mouse_down,
      .iMouse_w = 0.0,
      
      .iResolution_x = HARUKO_WIDTH,
      .iResolution_y = HARUKO_HEIGHT,
      
      .iTime = time
    };
    
    glBindBuffer(GL_UNIFORM_BUFFER, haruko.ubo_input);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ub_input_t), &ub_input);
    
    frame_update(iChannel);
    
    glUseProgram(image_shader);
    quad_draw();
    
    time += 0.015;
    
    SDL_GL_SwapWindow(haruko.window);
  }
  
  haruko_quit();
  
  return 0;
}

void haruko_init_input()
{
  glGenBuffers(1, &haruko.ubo_input);
  glBindBuffer(GL_UNIFORM_BUFFER, haruko.ubo_input);
  glBufferData(GL_UNIFORM_BUFFER, 512, NULL, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, haruko.ubo_input);
}

frame_t frame_image(const char *image_path)
{
  frame_t frame = {
    .type = FRAME_IMAGE,
    .image_path = image_path
  };
  
  return frame;
}

frame_t frame_shader(const char *shader_path)
{
  frame_t frame = {
    .type = FRAME_SHADER,
    .shader_path = shader_path
  };
  
  return frame;
}

bool frame_init(frame_t frame[], GLuint shader)
{
  for (int i = 0; i < MAX_ICHANNEL; i++) {
    if (frame[i].type == FRAME_IMAGE) {
      if (!frame_image_load(frame, i, shader)) {
        return false;
      }
    } else if (frame[i].type == FRAME_SHADER) {
      if (!frame_shader_load(frame, i, shader)) {
        return false;
      }
    }
  }
  
  for (int i = 0; i < MAX_ICHANNEL; i++) {
    if (frame[i].type == FRAME_SHADER) {
      frame_shader_setup(frame[i].shader, frame);
    }
  }
  
  return true;
}

bool frame_image_load(frame_t frame[], int num, GLuint shader)
{
  if (!haruko_load_image(&frame[num].texture, frame[num].image_path)) {
    return false;
  }
  
  glUseProgram(shader);
  
  return true;
}

bool frame_shader_load(frame_t frame[], int num, GLuint shader)
{
  glGenTextures(1, &frame[num].texture);
  glBindTexture(GL_TEXTURE_2D, frame[num].texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, HARUKO_WIDTH, HARUKO_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  glGenFramebuffers(1, &frame[num].fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, frame[num].fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame[num].texture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  if (!haruko_load_shader(&frame[num].shader, frame, frame[num].shader_path)) {
    return false;
  }
  
  glUseProgram(shader);
  
  return true;
}

void frame_update(frame_t frame[])
{
  for (int i = 0; i < MAX_ICHANNEL; i++) {
    if (frame[i].type == FRAME_SHADER) {
      glBindFramebuffer(GL_FRAMEBUFFER, frame[i].fbo);
      glClear(GL_COLOR_BUFFER_BIT);
      glUseProgram(frame[i].shader);
      quad_draw();
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
  }
}

void frame_shader_setup(GLuint shader, frame_t frame[])
{
  for (int i = 0; i < MAX_ICHANNEL; i++) {
    switch (frame[i].type) {
    case FRAME_IMAGE:
    case FRAME_SHADER:
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, frame[i].texture);
    case FRAME_CUBEMAP:
      break;
    default:
      break;
    }
  }
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

bool haruko_load_shader(GLuint *shader, frame_t frame[], const char *path)
{
  shader_setup_t shader_setup;
  shader_setup_init(&shader_setup, "shader");
  shader_setup_add(&shader_setup, SHADER_BOTH, "#version 300 es\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "precision mediump float;\n");
  
  shader_setup_add(&shader_setup, SHADER_BOTH, "layout (std140) uniform ub_input {\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "  vec4 iMouse;\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "  vec2 iResolution;\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "  float iTime;\n");
  shader_setup_add(&shader_setup, SHADER_BOTH, "};\n");
  
  for (int i = 0; i < MAX_ICHANNEL; i++) {
    char iChannel_name[32];
    
    switch (frame[i].type) {
    case FRAME_IMAGE:
    case FRAME_SHADER:
      snprintf(iChannel_name, 32, "uniform sampler2D iChannel%i;\n", i);
      shader_setup_add(&shader_setup, SHADER_FRAGMENT, iChannel_name);
      break;
    case FRAME_CUBEMAP:
      snprintf(iChannel_name, 32, "uniform samplerCube iChannel%i;\n", i);
      shader_setup_add(&shader_setup, SHADER_FRAGMENT, iChannel_name);
      break;
    default:
      break;
    }
  }
  
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
  
  for (int i = 0; i < MAX_ICHANNEL; i++) {
    char iChannel_name[32];
    snprintf(iChannel_name, 32, "iChannel%i", i);
    
    GLuint ul_iChannel = glGetUniformLocation(*shader, iChannel_name);
    glUniform1i(ul_iChannel, i);
  }
  
  GLuint ubl_input = glGetUniformBlockIndex(*shader, "ub_input");
  glUniformBlockBinding(*shader, ubl_input, 0);
  
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
