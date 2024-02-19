#ifndef LEX_H
#define LEX_H

#include <stdbool.h>

typedef enum {
  TK_EOF = 256,
  
  // - keyword -
  TK_BUFFER,
  TK_LOAD_IMAGE,
  TK_LOAD_CUBEMAP,
  TK_ICHANNEL0,
  TK_ICHANNEL1,
  TK_ICHANNEL2,
  TK_ICHANNEL3,
  
  // - symbol -
  TK_LEFT_ARROW,
  
  // - text -
  TK_ALIAS,
  TK_TEXT
} type_token_t;

typedef struct token_s {
  type_token_t type_token;
  char *text;
  struct token_s *next;
  const char *src;
  int line;
} token_t;

typedef struct {
  token_t *token;
  token_t *now;
} lex_t;

bool lex_parse_file(lex_t *lex, const char *path);
bool lex_match(lex_t *lex, type_token_t type_token);
token_t *lex_eat(lex_t *lex);
token_t *lex_accept(lex_t *lex, type_token_t type_token);
token_t *lex_expect(lex_t *lex, type_token_t type_token);
void lex_dump(lex_t *lex);

void print_token(const token_t *token);
void print_type_token(type_token_t type_token);

#endif
