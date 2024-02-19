#include "lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "file.h"

typedef struct {
  const char *src;
  int line;
  char *text;
  char *c;
} lex_file_t;

token_t *token_read_all(lex_file_t *lex);
token_t *scan_token(lex_file_t *lex);
token_t *token_match_alias(lex_file_t *lex);
token_t *token_match_eof(lex_file_t *lex);
token_t *token_match_text(lex_file_t *lex);
token_t *token_match_symbol(lex_file_t *lex);

bool skip_whitespace(lex_file_t *lex);

bool lex_parse_file(lex_t *lex, const char *path)
{
  char *text = file_read_all(path);
  
  if (!text) {
    return false;
  }
  
  lex_file_t lex_file = {
    .src = path,
    .line = 1,
    .text = text,
    .c = text
  };
  
  token_t *token = token_read_all(&lex_file);
  
  free(text);
  
  lex->token = token;
  lex->now = token;
  
  return true;
}

void lex_dump(lex_t *lex)
{
  token_t *now = lex->token;
  
  while (now) {
    printf("> TOKEN(");
    print_type_token(now->type_token);
    printf("): ");
    print_token(now);
    printf("\n");
    now = now->next;
  }
}

bool lex_match(lex_t *lex, type_token_t type_token)
{
  return lex->now->type_token == type_token;
}

token_t *lex_eat(lex_t *lex)
{
  token_t *token = lex->now;
  lex->now = lex->now->next;
  return token;
}

token_t *lex_accept(lex_t *lex, type_token_t type_token)
{
  if (lex_match(lex, type_token)) {
    return lex_eat(lex);
  }
  
  return NULL;
}

token_t *lex_expect(lex_t *lex, type_token_t type_token)
{
  token_t *token = lex_accept(lex, type_token);
  
  if (!token) {
    printf("%s:%i: expected '", lex->now->src, lex->now->line);
    print_type_token(type_token);
    printf("' but found '");
    print_token(lex->now);
    printf("'.\n");
    exit(1);
  }
  
  return token;
}

void print_token(const token_t *token)
{
  if (token->type_token < TK_EOF) {
    printf("%c", token->type_token);
  } else if (token->type_token  == TK_EOF) {
    printf("EOF");
  } else if (token->type_token < TK_ALIAS) {
    const char *keyword[] = {
      "buffer",
      "load_image",
      "load_cubemap",
      "iChannel0",
      "iChannel1",
      "iChannel2",
      "iChannel3",
      "<-"
    };
    
    printf("%s", keyword[token->type_token - TK_BUFFER]);
  } else if (token->type_token == TK_ALIAS) {
    printf("%s", token->text);
  } else if (token->type_token == TK_TEXT) {
    printf("\"%s\"", token->text);
  }
}

void print_type_token(type_token_t type_token)
{
  if (type_token < TK_EOF) {
    printf("%c", type_token);
  } else if (type_token <= TK_TEXT) {
    const char *type_token_text[] = {
      "EOF",
      "buffer",
      "load_image",
      "load_cubemap",
      "iChannel0",
      "iChannel0",
      "iChannel0",
      "iChannel0",
      "<-",
      "alias",
      "text"
    };
    
    printf("%s", type_token_text[type_token - TK_EOF]);
  }
}

token_t *token_read_all(lex_file_t *lex)
{
  token_t *tail = scan_token(lex);
  token_t *head = tail;
  
  while (tail->type_token != TK_EOF) {
    tail->next = scan_token(lex);
    tail = tail->next;
  }
  
  return head;
}

token_t *scan_token(lex_file_t *lex)
{
  while (skip_whitespace(lex));
  
  token_t *token = NULL;
  
  while (!token) {
    if ((token = token_match_alias(lex))) return token;
    if ((token = token_match_eof(lex))) return token;
    if ((token = token_match_text(lex))) return token;
    if ((token = token_match_symbol(lex))) return token;
    
    if (isprint(*lex->c)) {
      printf("warning: skipping unknown character: '%c' (%i)\n", *lex->c, *lex->c);
    } else {
      printf("warning: skipping unknown character: (%i)\n", *lex->c, *lex->c);
    }
    lex->c++;
  }
  
  return token;
}

bool skip_whitespace(lex_file_t *lex)
{
  switch (*lex->c) {
  case '\n':
    lex->line++;
  case ' ':
  case '\t':
    lex->c++;
    return true;
  }
  
  return false;
}

token_t *token_match_symbol(lex_file_t *lex)
{
  const char *symbol[] = {
    "<-",
    "{",
    "}",
    ";",
    ","
  };
  
  int num_symbol = sizeof(symbol) / sizeof(const char *);
  
  for (int i = 0; i < num_symbol; i++) {
    int length = strlen(symbol[i]);
    
    if (strncmp(lex->c, symbol[i], length) == 0) {
      token_t *token = malloc(sizeof(token_t));
      
      type_token_t type_token = *lex->c;
      
      if (length > 1) {
        type_token = TK_LEFT_ARROW + i;
      }
      
      *token = (token_t) {
        .type_token = type_token,
        .text = NULL,
        .next = NULL,
        .src = lex->src,
        .line = lex->line
      };
      
      lex->c += length;
      
      return token;
    }
  }
  
  return NULL;
}

token_t *token_match_alias(lex_file_t *lex)
{
  if (!isalpha(*lex->c) && *lex->c != '_') {
    return NULL;
  }
  
  char *end = lex->c;
  
  while (isalnum(*end) || *end == '_') {
    end++;
  }
  
  int length = end - lex->c;
  
  const char *keyword[] = {
    "buffer",
    "load_image",
    "load_cubemap",
    "iChannel0",
    "iChannel1",
    "iChannel2",
    "iChannel3"
  };
  
  int num_keyword = sizeof(keyword) / sizeof(const char*);
  
  for (int i = 0; i < num_keyword; i++) {
    if (strncmp(lex->c, keyword[i], length) == 0) {
      token_t *token = malloc(sizeof(token_t));
      
      *token = (token_t) {
        .type_token = TK_BUFFER + i,
        .text = NULL,
        .next = NULL,
        .src = lex->src,
        .line = lex->line
      };
      
      lex->c += length;
      
      return token;
    }
  }
  
  char *text = calloc(length, sizeof(char));
  memcpy(text, lex->c, length);
  text[length] = 0;
  
  token_t *token = malloc(sizeof(token_t));
  
  *token = (token_t) {
    .type_token = TK_ALIAS,
    .text = text,
    .next = NULL,
    .src = lex->src,
    .line = lex->line
  };
  
  lex->c += length;
  
  return token;
}

token_t *token_match_text(lex_file_t *lex)
{
  if (*lex->c != '"') {
    return NULL;
  }
  
  char *end = lex->c + 1;
  int line = lex->line;
  
  while (*end != '"') {
    if (*end == 0) {
      fprintf(stderr, "error: unescaped string at line %i", line);
      exit(1);
    }
    
    if (*end == '\n') {
      lex->line++;
    }
    
    end++;
  }
  
  int length = end - lex->c - 1;
  
  char *text = calloc(length, sizeof(char));
  memcpy(text, lex->c + 1, length);
  text[length] = 0;
  
  token_t *token = malloc(sizeof(token_t));
  
  *token = (token_t) {
    .type_token = TK_TEXT,
    .text = text,
    .next = NULL,
    .src = lex->src,
    .line = line
  };
  
  lex->c += 2 + length;
  
  return token;
}

token_t *token_match_eof(lex_file_t *lex)
{
  if (*lex->c != 0) {
    return NULL;
  }
  
  token_t *token = malloc(sizeof(token_t));
  token->type_token = TK_EOF;
  token->text = NULL;
  token->src = lex->src;
  token->line = lex->line;
  
  return token;
}
