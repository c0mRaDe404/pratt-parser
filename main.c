#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define error(s) fprintf(stderr, s);

typedef enum { TERROR, TNUM, TPLUS, TMINUS, TSTAR, TSLASH, TEOF } T_type;

typedef struct {
  T_type type;
  char lexeme;
} Token;

typedef struct {
  const char *head;
} LexState;

typedef struct {
  LexState *L;
  Token ctok;
  Token ptok;
} ParseState;

typedef struct {
  float lbp;
  float rbp;
} bp;

typedef enum { ANUM, AADD, ASUB, ADIV, AMUL, AERR } ast_type;

typedef struct ast_ {
  ast_type type;
  union {
    int num;
    struct {
      struct ast_ *left;
      struct ast_ *right;
    } binary;
  };
} ast_;

Token get_tok(LexState *L) {
  char c = *L->head++;
  switch (c) {
  case '+':
    return (Token){TPLUS, c};
  case '-':
    return (Token){TMINUS, c};
  case '*':
    return (Token){TSTAR, c};
  case '/':
    return (Token){TSLASH, c};
  case '\0':
    return (Token){TEOF, c};
  default:
    if (isdigit(c)) {
      return (Token){TNUM, c};
    } else {
      error("Unrecognized Token");
      break;
    }
  }
  return (Token){0};
}

Token next(ParseState *P) { return P->ctok = get_tok(P->L); }

Token cur(ParseState *P) { return P->ctok; }

Token peek(ParseState *P) {
  Token t = get_tok(P->L);
  P->L->head--;
  return t;
}

bp binding_power(T_type op) {
  switch (op) {
  case TPLUS:
  case TMINUS:
    return (bp){1, 1.1};
  case TSTAR:
  case TSLASH:
    return (bp){2, 2.1};
  default:
    return (bp){0};
  }
}

int num(ParseState *P) { return next(P).lexeme - '0'; }

int is_op(T_type op) {
  switch (op) {
  case TPLUS:
  case TMINUS:
  case TSTAR:
  case TSLASH:
    return 1;
  default:
    return 0;
  }
}

ast_ *mk_numNode(int num) {
  ast_ *n = malloc(sizeof *n);
  n->type = ANUM;
  n->num = num;
  return n;
}

ast_ *mk_binaryNode(int op, ast_ *left, ast_ *right) {
  ast_ *n = malloc(sizeof *n);
  n->type = op;
  n->binary.left = left;
  n->binary.right = right;
  return n;
}

ast_type ttoa(T_type tok) {
  switch (tok) {
  case TPLUS:
    return AADD;
  case TMINUS:
    return ASUB;
  case TSTAR:
    return AMUL;
  case TSLASH:
    return ADIV;
  default:
    return AERR;
  }
}

void advance(ParseState *P) { P->ctok = get_tok(P->L); }

ast_ *parse_expression(ParseState *P, float min_bp) {
  ast_ *lhs = mk_numNode(num(P));
  while (is_op(peek(P).type)) {
    Token tok = peek(P);
    bp bpow = binding_power(tok.type);
    if (min_bp > bpow.lbp)
      return lhs;
    advance(P);
    lhs = mk_binaryNode(ttoa(tok.type), lhs, parse_expression(P, bpow.rbp));
  }
  return lhs;
}

ast_ *parser(LexState *L) {
  ParseState P = {L, {0}};
  return parse_expression(&P, 0);
}

int interpret(ast_ *n) {

  int lhs;
  int rhs;
  int num;
  if (n->type != ANUM) {
    lhs = interpret(n->binary.left);
    rhs = interpret(n->binary.right);
    free(n);
  } else {
    num = n->num;
    free(n);
    return num;
  }
  switch (n->type) {
  case AADD:
    return lhs + rhs;
  case ASUB:
    return lhs - rhs;
  case AMUL:
    return lhs * rhs;
  case ADIV:
    return lhs / rhs;
  default:
    break;
  }
  return 0;
}

int main(void) {
  char *exp = "3+4-5*2*2*3*1+9*3*3+8";
  LexState L = {exp};
  ast_ *root = parser(&L);
  int result = interpret(root);
  printf("%d\n", result);
}
