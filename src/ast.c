#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

typedef ast_t *(rule_t)(lex_t *lex);

ast_t *ast_stmt(lex_t *lex);
ast_t *ast_stmt_body(lex_t *lex);
ast_t *ast_load_image(lex_t *lex);
ast_t *ast_load_cubemap(lex_t *lex);
ast_t *ast_buffer(lex_t *lex);
ast_t *ast_set_channel_list(lex_t *lex);
ast_t *ast_set_channel(lex_t *lex);
token_t *token_channel(lex_t *lex);

ast_t *expect_rule(lex_t *lex, rule_t *rule, const char *name);

ast_t *ast_parse(lex_t *lex)
{
  ast_t *head = expect_rule(lex, ast_stmt, "statement");
  ast_t *tail = head;
  
  while (!lex_accept(lex, TK_EOF)) {
    tail->stmt.next = expect_rule(lex, ast_stmt, "statement");
    tail = tail->stmt.next;
  }
  
  return head;
}

ast_t *ast_stmt(lex_t *lex)
{
  ast_t *body = ast_stmt_body(lex);
  
  if (!body) {
    return NULL;
  }
  
  ast_t *ast = malloc(sizeof(ast_t));
  
  *ast = (ast_t) {
    .type_ast = AST_STMT,
    .stmt = {
      .body = body,
      .next = NULL
    }
  };

  return ast;
}

ast_t *ast_stmt_body(lex_t *lex)
{
  ast_t *body = NULL;
  
  if ((body = ast_load_image(lex))) return body;
  if ((body = ast_load_cubemap(lex))) return body;
  if ((body = ast_buffer(lex))) return body;
  
  return NULL;
}

ast_t *ast_load_image(lex_t *lex)
{
  if (!lex_accept(lex, TK_LOAD_IMAGE)) {
    return NULL;
  }
  
  token_t *alias = lex_expect(lex, TK_ALIAS);
  token_t *path = lex_expect(lex, TK_TEXT);
  
  lex_expect(lex, ';');
  
  ast_t *ast = malloc(sizeof(ast_t));
  
  *ast = (ast_t) {
    .type_ast = AST_LOAD_IMAGE,
    .load_image = {
      .alias = alias,
      .path = path
    }
  };
  
  return ast;
}

ast_t *ast_load_cubemap(lex_t *lex)
{
  if (!lex_accept(lex, TK_LOAD_CUBEMAP)) {
    return NULL;
  }
  
  token_t *alias = lex_expect(lex, TK_ALIAS);
  token_t *path = lex_expect(lex, TK_TEXT);
  token_t *ext = lex_expect(lex, TK_TEXT);
  
  lex_expect(lex, ';');
  
  ast_t *ast = malloc(sizeof(ast_t));
  
  *ast = (ast_t) {
    .type_ast = AST_LOAD_CUBEMAP,
    .load_cubemap = {
      .alias = alias,
      .path = path,
      .ext = ext
    }
  };
  
  return ast;
}

ast_t *ast_buffer(lex_t *lex)
{
  if (!lex_accept(lex, TK_BUFFER)) {
    return NULL;
  }
  
  token_t *alias = lex_expect(lex, TK_ALIAS);
  token_t *path = lex_expect(lex, TK_TEXT);
  
  ast_t *set_channel = NULL;
  
  if (lex_accept(lex, '{')) {
    set_channel = ast_set_channel_list(lex);
    lex_expect(lex, '}');
  }
  
  lex_expect(lex, ';');
  
  ast_t *ast = malloc(sizeof(ast_t));
  
  *ast = (ast_t) {
    .type_ast = AST_BUFFER,
    .buffer = {
      .alias = alias,
      .path = path,
      .set_channel = set_channel
    }
  };
  
  return ast;
}

ast_t *ast_set_channel_list(lex_t *lex)
{
  ast_t *head = ast_set_channel(lex);
  ast_t *tail = head;
  
  if (!head) {
    return NULL;
  }
  
  while (lex_accept(lex, ',')) {
    tail->set_channel.next = expect_rule(lex, ast_set_channel, "set-channel");
    tail = tail->set_channel.next;
  }
  
  return head;
}

ast_t *ast_set_channel(lex_t *lex)
{
  token_t *channel = token_channel(lex);
  
  if (!channel) {
    return NULL;
  }
  
  lex_expect(lex, TK_LEFT_ARROW);
  
  token_t *alias = lex_expect(lex, TK_ALIAS);
  
  ast_t *ast = malloc(sizeof(ast_t));
  
  *ast = (ast_t) {
    .type_ast = AST_SET_CHANNEL,
    .set_channel = {
      .channel = channel,
      .alias = alias,
      .next = NULL
    }
  };
  
  return ast;
}

token_t *token_channel(lex_t *lex)
{
  token_t *channel = NULL;
  
  if ((channel = lex_accept(lex, TK_ICHANNEL0))) return channel;
  if ((channel = lex_accept(lex, TK_ICHANNEL1))) return channel;
  if ((channel = lex_accept(lex, TK_ICHANNEL2))) return channel;
  if ((channel = lex_accept(lex, TK_ICHANNEL3))) return channel;
  
  return NULL;
}

ast_t *expect_rule(lex_t *lex, rule_t *rule, const char *name)
{
  ast_t *ast = rule(lex);
  
  if (!ast) {
    printf("%s:%i: expected 'rule-%s' but found '", lex->now->src, lex->now->line, name);
    print_token(lex->now);
    printf("'.\n");
    exit(1);
  }
  
  return ast;
}

void print_ast_stmt(ast_t *ast);
void print_ast_buffer(ast_t *ast);

void print_ast(ast_t *ast)
{
  switch (ast->type_ast) {
  case AST_STMT:
    print_ast_stmt(ast);
    break;
  case AST_LOAD_IMAGE:
    printf("load_image ");
    print_token(ast->load_image.alias);
    printf(" ");
    print_token(ast->load_image.path);
    printf(";");
    break;
  case AST_LOAD_CUBEMAP:
    printf("load_cubemap ");
    print_token(ast->load_cubemap.alias);
    printf(" ");
    print_token(ast->load_cubemap.path);
    printf(" ");
    print_token(ast->load_cubemap.ext);
    printf(";");
    break;
  case AST_BUFFER:
    print_ast_buffer(ast);
    break;
  case AST_SET_CHANNEL:
    print_token(ast->set_channel.channel);
    printf(" <- ");
    print_token(ast->set_channel.alias);
    break;
  default:
    printf("ast:unknown_case(%i)", ast->type_ast);
    break;
  }
}

void print_ast_stmt(ast_t *ast)
{
  print_ast(ast->stmt.body);
  if (ast->stmt.next) {
    printf("\n");
    print_ast(ast->stmt.next);
  }
}

void print_ast_buffer(ast_t *ast)
{
  printf("buffer ");
  print_token(ast->buffer.alias);
  printf(" ");
  print_token(ast->buffer.path);
  
  ast_t *set_channel = ast->buffer.set_channel;
  
  if (set_channel) {
    printf(" {\n");
    while (set_channel) {
      printf("  ");
      print_ast(set_channel);
      set_channel = set_channel->set_channel.next;
      
      if (set_channel) {
        printf(",\n");
      }
    }
    printf("\n}");
  }
  
  printf(";");
}

token_t *ast_token(ast_t *ast)
{
  switch (ast->type_ast) {
  case AST_STMT:
    return ast_token(ast->stmt.body);
    break;
  case AST_LOAD_IMAGE:
    return ast->load_image.alias;
    break;
  case AST_LOAD_CUBEMAP:
    return ast->load_cubemap.alias;
    break;
  case AST_BUFFER:
    return ast->buffer.alias;
    break;
  case AST_SET_CHANNEL:
    return ast->set_channel.channel;
  default:
    return NULL;
  }
}
