#include "setup.h"

#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "haruko.h"
#include "path.h"

#define MAX_VAR 32

typedef struct {
  buffer_t *buffer;
  channel_t channel;
  char *name;
} var_t;

struct {
  path_t src;
  ast_t *ast;
  var_t var[MAX_VAR];
  int num_var;
} setup;

void setup_error(ast_t *ast, const char *format, ...);
bool setup_ast(ast_t *ast);
bool setup_buffer_init(ast_t *ast);
bool setup_buffer_setup(ast_t *ast);
bool setup_stmt(ast_t *ast);
bool setup_buffer(ast_t *ast);
bool setup_buffer_load(ast_t *ast);
bool setup_load_image(ast_t *ast);
bool setup_load_cubemap(ast_t *ast);

var_t *setup_var_find(const char *name);
bool setup_var_insert(const char *name, GLuint type, GLuint texture, buffer_t *buffer);
buffer_t *setup_buffer_find(const char *name);

bool setup_load(const char *path)
{
  path_create(setup.src, path);
  
  lex_t lex;
  
  if (!lex_parse_file(&lex, setup.src)) {
    return false;
  }
  
  setup.ast = ast_parse(&lex);
  
  return true;
}

bool setup_run()
{
  setup_ast(setup.ast);
}

bool setup_ast(ast_t *ast)
{
  buffer_t *image = haruko_get_image();
  
  if (!setup_var_insert("image", GL_TEXTURE_2D, image->texture, image)) {
    return false;
  }
  
  assert(ast->type_ast == AST_STMT);
  
  setup_buffer_init(ast);
  setup_buffer_setup(ast);
  
  return true;
}

bool setup_buffer_init(ast_t *ast)
{
  ast_t *now = ast;
  
  while (now) {
    if (!setup_stmt(now->stmt.body)) {
      return false;
    }
    
    now = now->stmt.next;
  }
  
  return true;
}

bool setup_buffer_setup(ast_t *ast)
{
  ast_t *now = ast;
  
  while (now) {
    if (!setup_buffer_load(now->stmt.body)) {
      return false;
    }
    
    now = now->stmt.next;
  }
  
  return true;
}

bool setup_stmt(ast_t *ast)
{
  switch (ast->type_ast) {
  case AST_BUFFER:
    return setup_buffer(ast);
  case AST_LOAD_IMAGE:
    return setup_load_image(ast);
  case AST_LOAD_CUBEMAP:
    return setup_load_cubemap(ast);
  default:
    fprintf(stderr, "unknown type_ast_t(%i)", ast->type_ast);
    return false;
  }
}

bool setup_buffer(ast_t *ast)
{
  buffer_t *buffer = NULL;
  
  const char *name = ast->buffer.alias->text;
  
  if (strcmp(name, "image") == 0) {
    buffer = haruko_get_image();
  } else {
    buffer = haruko_add_buffer();
  
    if (!setup_var_insert(name, GL_TEXTURE_2D, buffer->texture, buffer)) {
      setup_error(ast, "buffer with name '%s' already exists.", name);
      return false;
    }
  }
  
  return true;
}

bool setup_load_image(ast_t *ast)
{
  path_t path;
  path_copy(path, setup.src);
  path_new(path, ast->load_image.path->text);
  
  GLuint texture;
  if (!haruko_load_image(&texture, path)) {
    setup_error(ast, "failed to load image '%s'.", path);
    return false;
  }
  
  if (!setup_var_insert(ast->load_image.alias->text, GL_TEXTURE_2D, texture, NULL)) {
    return false;
  }
  
  return true;
}

bool setup_load_cubemap(ast_t *ast)
{
  path_t path[6];
  
  const char *face_name[] = {
    "right",
    "left",
    "up",
    "down",
    "front",
    "back"
  };
  
  for (int i = 0; i < 6; i++) {
    path_t base;
    path_copy(base, setup.src);
    path_new(base, ast->load_cubemap.path->text);
    path_create(path[i], "%s/%s.%s", base, face_name[i], ast->load_cubemap.ext->text);
  }
  
  const char *faces[] = {
    path[0], path[1], path[2],
    path[3], path[4], path[5]
  };
  
  GLuint texture;
  if (!haruko_load_cubemap(&texture, faces)) {
    return false;
  }
  
  const char *name = ast->load_image.alias->text;
  
  if (!setup_var_insert(name, GL_TEXTURE_CUBE_MAP, texture, NULL)) {
    return false;
  }
  
  return true;
}

bool setup_buffer_load(ast_t *ast)
{
  if (ast->type_ast != AST_BUFFER) {
    return true;
  }
  
  buffer_t *buffer = setup_buffer_find(ast->buffer.alias->text);
  
  if (!buffer) {
    return false;
  }
  
  ast_t *now = ast->buffer.set_channel;
  
  while (now) {
    const char *name = now->set_channel.alias->text;
    var_t *var = setup_var_find(name);
    
    if (!var) {
      setup_error(ast, "buffer '%s' does not exist.", name);
      return false;
    }
    
    int channel = now->set_channel.channel->type_token - TK_ICHANNEL0;
    buffer->channel[channel] = var->channel;
    now = now->set_channel.next;
  }
  
  path_t path;
  path_copy(path, setup.src);
  path_new(path, ast->buffer.path->text);
  
  if (!buffer_shader_load(buffer, path)) {
    return false;
  }
  
  return true;
}

var_t *setup_var_find(const char *name)
{
  for (int i = 0; i < setup.num_var; i++) {
    if (strcmp(name, setup.var[i].name) == 0) {
      return &setup.var[i];
    }
  }
  
  return NULL;
}

buffer_t *setup_buffer_find(const char *name)
{
  var_t *var = setup_var_find(name);
  
  if (!var) {
    return false;
  }
  
  return var->buffer;
}

bool setup_var_insert(const char *name, GLuint type, GLuint texture, buffer_t *buffer)
{
  if (setup_var_find(name)) {
    return false;
  }
  
  var_t *var = &setup.var[setup.num_var++];
  var->name = strdup(name);
  var->buffer = buffer;
  channel_init(&var->channel, type, texture);
  
  return true;
}

void setup_error(ast_t *ast, const char *format, ...)
{
  if (!ast) {
    return;
  }
  
  va_list args;
  va_start(args, format);
  token_t *token = ast_token(ast);
  fprintf(stderr, "%s:%i: ", token->src, token->line);
  vfprintf(stderr, format, args);
  va_end(args);
  
  putc('\n', stdout);
}
