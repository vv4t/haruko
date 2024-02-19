#ifndef AST_H
#define AST_H

#include "lex.h"

typedef enum {
  AST_STMT,
  AST_LOAD_IMAGE,
  AST_LOAD_CUBEMAP,
  AST_BUFFER,
  AST_SET_CHANNEL
} type_ast_t;

typedef struct ast_s {
  type_ast_t type_ast;
  union {
    struct {
      struct ast_s *body;
      struct ast_s *next;
    } stmt;
    struct {
      token_t *alias;
      token_t *path;
    } load_image;
    struct {
      token_t *alias;
      token_t *path;
      token_t *ext;
    } load_cubemap;
    struct {
      token_t *alias;
      token_t *path;
      struct ast_s *set_channel;
    } buffer;
    struct {
      token_t *channel;
      token_t *alias;
      struct ast_s *next;
    } set_channel;
  };
} ast_t;

ast_t *ast_parse(lex_t *lex);
token_t *ast_token(ast_t *ast);
void print_ast(ast_t *ast);

#endif
