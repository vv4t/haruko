#include <stdio.h>

#define HARUKO_WIDTH 640
#define HARUKO_HEIGHT 360

#define MAX_CHANNEL 4

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "file.h"
#include "gl.h"
#include "quad.h"
#include "shader.h"
#include "shader_setup.h"

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
  
  GLuint ubo_input;
  
  channel_t empty_channel;
} haruko;

void haruko_poll();
void haruko_quit();
bool haruko_load_sdl(const char *title, int width, int height);
bool haruko_load_image(GLuint *texture, const char *image_path);
bool haruko_load_cubemap(GLuint *texture, const char *faces[]);
void haruko_init_input();
void haruko_init_empty_channel();

void channel_init(channel_t *channel, GLuint type, GLuint texture);

buffer_t buffer_default();
buffer_t buffer_new();
bool buffer_shader_load(buffer_t *buffer, const char *shader_path);
void buffer_update(buffer_t *buffer);

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
  haruko_init_empty_channel();
  
  buffer_t image = buffer_default();
  
  buffer_t buffer[] = {};
  int num_buffer = sizeof(buffer) / sizeof(buffer_t);
  
  const char *faces[] = {
    "shader/skybox/right.jpg",
    "shader/skybox/left.jpg",
    "shader/skybox/up.jpg",
    "shader/skybox/down.jpg",
    "shader/skybox/back.jpg",
    "shader/skybox/front.jpg"
  };
  
  GLuint cubemap;
  
  if (!haruko_load_cubemap(&cubemap, faces)) {
    return false;
  }
  
  channel_init(&image.channel[0], GL_TEXTURE_CUBE_MAP, cubemap);
  
  if (!buffer_shader_load(&image, "shader/skybox/image.glsl")) return false;
  
  quad_bind();
  
  float time = 0.0;
  
  while (!haruko.quit) {
    haruko_poll();
    
    ub_input_t ub_input = {
      .iMouse_x = haruko.mouse_x,
      .iMouse_y = haruko.mouse_y,
      .iMouse_z = haruko.click_x * (haruko.mouse_down > 0.0 ? 1.0 : -1.0),
      .iMouse_w = haruko.click_y * (haruko.mouse_click > 0.0 ? 1.0 : -1.0),
      
      .iResolution_x = HARUKO_WIDTH,
      .iResolution_y = HARUKO_HEIGHT,
      .iResolution_z = 1.0,
      
      .iTime = time
    };
    
    haruko.mouse_click = 0.0;
    
    glBindBuffer(GL_UNIFORM_BUFFER, haruko.ubo_input);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ub_input_t), &ub_input);
    
    for (int i = 0; i < num_buffer; i++) {
      buffer_update(&buffer[i]);
    }
    
    buffer_update(&image);
    
    time += 0.015;
    
    SDL_GL_SwapWindow(haruko.window);
  }
  
  haruko_quit();
  
  return 0;
}

void channel_init(channel_t *channel, GLuint type, GLuint texture)
{
  channel->type = type;
  channel->texture = texture;
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

buffer_t buffer_default()
{
  buffer_t buffer = {
    .fbo = 0,
    .texture = 0,
    .channel = {
      haruko.empty_channel,
      haruko.empty_channel,
      haruko.empty_channel,
      haruko.empty_channel
    }
  };
  
  return buffer;
}

buffer_t buffer_new()
{
  buffer_t buffer;
  
  glGenTextures(1, &buffer.texture);
  glBindTexture(GL_TEXTURE_2D, buffer.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, HARUKO_WIDTH, HARUKO_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  glGenFramebuffers(1, &buffer.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.texture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  buffer.channel[0] = haruko.empty_channel;
  buffer.channel[1] = haruko.empty_channel;
  buffer.channel[2] = haruko.empty_channel;
  buffer.channel[3] = haruko.empty_channel;
  
  return buffer;
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

bool haruko_load_cubemap(GLuint *texture, const char *faces[])
{
  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, *texture);
  
  for (int i = 0; i < 6; i++) {
    SDL_Surface *bitmap = IMG_Load(faces[i]);
    
    if (!bitmap) {
      fprintf(stderr, "could not load %s", faces[i]);
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
