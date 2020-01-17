#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "token.h"
#include "misc.h"

char *token_names[38] = {
	"TK_ASTERISK", "TK_POINTSTO", "TK_DOT", "TK_SEMICOLON", "TK_COMMA",
	"TK_DPLUS", "TK_PLUS", "TK_DMINUS", "TK_MINUS",
	"TK_BEGIN", "TK_END",
	"TK_DIVIDE", "TK_MOD", "TK_ASSIGN",
	"TK_EQ", "TK_NEQ", "TK_LT", "TK_LEQ", "TK_GT", "TK_GEQ",
	"TK_AND", "TK_OR", "TK_DAND", "TK_DOR", "TK_EXCLAMATION",
	"TK_ELLIPSIS", "TK_TILDE", "TK_CARET",
	"TK_QUESTION", "TK_COLON",
	"TK_OPENPA", "TK_CLOSEPA", "TK_OPENBR", "TK_CLOSEBR",
	"TK_CINT", "TK_CCHAR", "TK_CSTR", "TK_IDENT",
};

int token_ids[38] = {
	TK_ASTERISK, TK_POINTSTO, TK_DOT, TK_SEMICOLON, TK_COMMA,
	TK_DPLUS, TK_PLUS, TK_DMINUS, TK_MINUS,
	TK_BEGIN, TK_END,
	TK_DIVIDE, TK_MOD, TK_ASSIGN,
	TK_EQ, TK_NEQ, TK_LT, TK_LEQ, TK_GT, TK_GEQ,
	TK_AND, TK_OR, TK_DAND, TK_DOR, TK_EXCLAMATION,
	TK_ELLIPSIS, TK_TILDE, TK_CARET,
	TK_QUESTION, TK_COLON,
	TK_OPENPA, TK_CLOSEPA, TK_OPENBR, TK_CLOSEBR,
	TK_CINT, TK_CCHAR, TK_CSTR, TK_IDENT,
};

char *keyword_names[32] = {
	"if", "else", "switch", "case", "default", "return", "goto",
	"for", "while", "do", "continue", "break",
	"void", "char", "int", "float", "double",
	"short", "long", "signed", "unsigned",
	"struct", "enum", "union",
	"typedef", "sizeof", "extern", "volatile", "const",
	"auto", "static", "register"
};

int keyword_ids[32] = {
	KW_IF, KW_ELSE, KW_SWITCH, KW_CASE, KW_DEFAULT, KW_RETURN, KW_GOTO,
	KW_FOR, KW_WHILE, KW_DO, KW_CONTINUE, KW_BREAK,
	KW_VIOD, KW_CHAR, KW_INT, KW_FLOAT, KW_DOUBLE,
	KW_SHORT, KW_LONG, KW_SIGNED, KW_UNSIGNED,
	KW_STRUCT, KW_ENUM, KW_UNION,
	KW_TYPEDEF, KW_SIZEOF, KW_EXTERN, KW_VOLATILE, KW_CONST,
	KW_AUTO, KW_STATIC, KW_REGISTER
};

struct token *new_token(int type, void *value)
{
	struct token *t;

	t = malloc(sizeof(struct token));
	if (!t)
		exit_with("Failed alloc memory for token %d\n", type);

	t->prev = NULL;
	t->next = NULL;

	t->type = type;
	t->value = value;

	t->fname = NULL;
	t->line = 0;

	return t;
}

struct token *copy_token(struct token *chain, struct token *orig)
{
	struct token *t;

	t = new_token(orig->type, orig->value);

	t->fname = orig->fname;
	t->line = orig->line;

	t->prev = chain;
	chain->next = t;

	return chain->next;
}

struct token *copy_token_chain(struct token *orig)
{
	struct token *r, *t;

	if (!orig)
		return NULL;

	t = new_token(0, NULL);
	r = t;
	for (; orig; orig = orig->next)
		r = copy_token(r, orig);

	r = t->next;
	free(t);

	return r;
}

void print_token_value_as_int(void *raw)
{
	intptr_t v;
	v = (intptr_t) raw;
	printf("0x%x, 0o%o, %d\n", v, v, v);
}

int print_token(struct token *t)
{
	printf("(%s:%d)%s ", t->fname, t->line, token_type_str(t->type));
	if (t->type != TK_CSTR && t->type != TK_IDENT)
		print_token_value_as_int(t->value);
	else
		printf("<%s>\n", t->value);

	return 1;
}

int print_token_list(struct token *start)
{
	struct token *p;

	for (p = start; p; p = p->next)
		print_token(p);

	printf("\n");
	return 1;
}

char *token_type_str(int type)
{
	int i;
	for (i = 0; i < 38; i++) {
		if (token_ids[i] == type)
			return token_names[i];
	}
	for (i = 0; i < 32; i++) {
		if (keyword_ids[i] == type)
			return keyword_names[i];
	}
	return "TK_UNKNOWN";
}

int try_get_keyword(char *s)
{
	int i;
	for (i = 0; i < 32; i++) {
		if (!strcmp(s, keyword_names[i]))
			return keyword_ids[i];
	}
	return 0;
}
