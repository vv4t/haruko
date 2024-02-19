#include "haruko.h"

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "file.h"
#include "gl.h"
#include "quad.h"
#include "buffer.h"

#define HARUKO_WIDTH 640
#define HARUKO_HEIGHT 360
#define MAX_BUFFER 32

typedef struct {
  float iMouse_x;
  float iMouse_y;
  float iMouse_z;
  float iMouse_w;
  
  float iResolution_x;
  float iResolution_y;
  float iResolution_z;
  
  float iTime;
} ub_input_t;

struct {
  SDL_Window *window;
  SDL_GLContext gl_context;
  
  bool quit;
  
  float mouse_x;
  float mouse_y;
  
  float click_x;
  float click_y;
  
  float mouse_down;
  float mouse_click;
  
  float time;
  
  buffer_t image;
  buffer_t buffer[MAX_BUFFER];
  int num_buffer;
  
  GLuint ubo_input;
  
  channel_t empty_channel;
} haruko;

void haruko_poll();
bool haruko_load_sdl(const char *title, int width, int height);
void haruko_init_input();
void haruko_init_empty_channel();
void haruko_update_input();

bool haruko_init()
{
  if (!haruko_load_sdl("haruko", HARUKO_WIDTH, HARUKO_HEIGHT)) {
    return false;
  }
  
  if (!gl_init()) {
    return false;
  }
  
  quad_init();
  haruko_init_input();
  haruko_init_empty_channel();
  
  haruko.time = 0.0;
  haruko.num_buffer = 0;
  
  haruko.quit = false;
  
  haruko.mouse_x = 0.0;
  haruko.mouse_y = 0.0;
  
  haruko.click_x = 0.0;
  haruko.click_y = 0.0;
  
  haruko.mouse_down = 0.0;
  haruko.mouse_click = 0.0;
  
  haruko.image = buffer_main(haruko.empty_channel);
  
  return true;
}

bool haruko_should_quit()
{
  return haruko.quit;
}

buffer_t *haruko_get_image()
{
  return &haruko.image;
}

buffer_t *haruko_add_buffer()
{
  buffer_t *buffer = &haruko.buffer[haruko.num_buffer++];
  *buffer = buffer_new(haruko.empty_channel, HARUKO_WIDTH, HARUKO_HEIGHT);
  return buffer;
}

void haruko_bind()
{
  quad_bind();
}

void haruko_update()
{
  haruko_poll();
  haruko_update_input();
  
  for (int i = 0; i < haruko.num_buffer; i++) {
    buffer_update(&haruko.buffer[i]);
  }
  
  buffer_update(&haruko.image);
  
  SDL_GL_SwapWindow(haruko.window);
  haruko.time += 0.015;
}

void haruko_update_input()
{
  ub_input_t ub_input = {
    .iMouse_x = haruko.mouse_x,
    .iMouse_y = haruko.mouse_y,
    .iMouse_z = haruko.click_x * (haruko.mouse_down > 0.0 ? 1.0 : -1.0),
    .iMouse_w = haruko.click_y * (haruko.mouse_click > 0.0 ? 1.0 : -1.0),
    
    .iResolution_x = HARUKO_WIDTH,
    .iResolution_y = HARUKO_HEIGHT,
    .iResolution_z = 1.0,
    
    .iTime = haruko.time
  };
  
  haruko.mouse_click = 0.0;
  
  glBindBuffer(GL_UNIFORM_BUFFER, haruko.ubo_input);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ub_input_t), &ub_input);
}

void haruko_init_input()
{
  glGenBuffers(1, &haruko.ubo_input);
  glBindBuffer(GL_UNIFORM_BUFFER, haruko.ubo_input);
  glBufferData(GL_UNIFORM_BUFFER, 512, NULL, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, haruko.ubo_input);
}

void haruko_init_empty_channel()
{
  float data[] = { 0.0, 0.0, 0.0, 1.0 };
  
  haruko.empty_channel.type = GL_TEXTURE_2D;
  
  glGenTextures(1, &haruko.empty_channel.texture);
  glBindTexture(GL_TEXTURE_2D, haruko.empty_channel.texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_FLOAT, data);
}

GLuint bitmap_format(SDL_Surface *bitmap);

bool haruko_load_image(GLuint *texture, const char *path)
{
  SDL_Surface *bitmap = IMG_Load(path);
  
  if (!bitmap) {
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

bool haruko_load_cubemap(GLuint *texture, const char *faces[])
{
  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, *texture);
  
  for (int i = 0; i < 6; i++) {
    SDL_Surface *bitmap = IMG_Load(faces[i]);
    
    if (!bitmap) {
      fprintf(stderr, "Failed to load image '%s'.\n", faces[i]);
      return false;
    }
    
    GLuint internal_format = bitmap_format(bitmap);

    glTexImage2D(
      GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format,
      bitmap->w, bitmap->h,
      0, internal_format, GL_UNSIGNED_BYTE, bitmap->pixels
    );
    
    SDL_FreeSurface(bitmap);
  }
  
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  
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
      haruko.click_x = event.button.x;
      haruko.click_y = HARUKO_HEIGHT - event.button.y;
      haruko.mouse_down = 1.0;
      haruko.mouse_click = 1.0;
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
    fprintf(stderr, "Failed to create SDL window.\n");
    return true;
  }
  
  haruko.gl_context = SDL_GL_CreateContext(haruko.window);
  
  if (!haruko.gl_context) {
    fprintf(stderr, "Failed to create GL context\n");
    return false;
  }
  
  return true;
}
