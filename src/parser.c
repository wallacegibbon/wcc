#include <stdlib.h>
#include "libc.h"
#include "misc.h"
#include "linktbl.h"
#include "token.h"
#include "node.h"
#include "ctypes.h"



struct parser { struct token *tk; struct linktbl *sdefs, *gvars, *funcs; };



int
check_unsupported_keyword(int type)
{
  return type == KW_REGISTER || type == KW_AUTO || type == KW_CONST ||
    type == KW_VOLATILE ||
    type == KW_STATIC ||
    type == KW_FLOAT || type == KW_DOUBLE ||
    type == KW_SWITCH ||
    type == KW_CASE ||
    type == KW_CONTINUE ||
    type == KW_BREAK ||
    type == KW_GOTO ||
    type == KW_TYPEDEF;
}


int
filter_unsupported_tk(struct token *tk)
{
  if (tk->type == TK_OPENBR || tk->type == TK_CLOSEBR)
    exit_with("%s:%d:[PARSER]Original C array is not supported by wcc\n",
        tk->fname, tk->line);

  if (check_unsupported_keyword(tk->type))
    exit_with("%s:%d:[PARSER]Unsupported keyword: %s\n",
        tk->fname, tk->line, token_type_str(tk->type));

  return 1;
}


struct token *
nexttoken(struct parser *psr)
{
  struct token *tk;

  if (!psr->tk)
    return NULL;

  tk = psr->tk->next;

  if (tk)
    filter_unsupported_tk(tk);

  psr->tk = tk;
  return tk;
}


struct token *
nexttoken_notend(struct parser *psr)
{
  struct token *tk;

  tk = nexttoken(psr);
  if (!tk)
    exit_with("[PARSER]Unexpected end of file\n");

  return tk;
}


int
adjust_int_extra(struct parser *psr)
{
  struct token *tk;

  tk = psr->tk;
  if ((tk->type == KW_SHORT || tk->type == KW_LONG) &&
      tk->next->type == KW_INT)
    nexttoken_notend(psr);

  return 1;
}


int
get_int_basic(struct parser *psr)
{
  struct token *tk;

  tk = psr->tk;
  adjust_int_extra(psr);

  nexttoken_notend(psr);

  if (tk->type == KW_CHAR)
    return CT_CHAR;

  if (tk->type == KW_SHORT)
    return CT_SHORT;

  if (tk->type == KW_INT)
    return CT_INT;

  if (tk->type == KW_LONG)
    return CT_LONG;

  exit_with("%s:%d:[PARSER]Invalid int variable declaration(%d)\n",
      tk->fname, tk->line, tk->type);

  return 1;
}


struct ctype *
get_unsigned_type(struct parser *psr)
{
  struct ctype *ct;
  int type;

  nexttoken_notend(psr);
  type = get_int_basic(psr);
  type |= 0x10;

  ct = new_ctype(type, 0, NULL, 0);
  return ct;
}


struct ctype *
get_signed_type(struct parser *psr)
{
  struct ctype *ct;
  int type;

  nexttoken_notend(psr);
  type = get_int_basic(psr);

  ct = new_ctype(type, 0, NULL, 0);
  return ct;
}


struct ctype *
get_void_type(struct parser *psr)
{
  struct ctype *ct;

  ct = new_ctype(CT_VOID, 0, NULL, 0);
  nexttoken_notend(psr);

  return ct;
}


struct ctype *
get_struct_type(struct parser *psr)
{
  struct ctype *ct;
  struct token *tk;
  char *name;

  tk = nexttoken_notend(psr);
  if (tk->type != TK_IDENT)
    exit_with("%s:%d:[PARSER]Name for struct not specified\n",
        tk->fname, tk->line);

  name = (void *) tk->value;

  nexttoken_notend(psr);

  ct = new_ctype(CT_STRUCT, 0, name, 0);

  return ct;
}


struct ctype *
get_basic_type(struct parser *psr)
{
  struct token *tk;
  struct ctype *ct;
  int type;

  tk = psr->tk;

  if (tk->type == KW_STRUCT)
    return get_struct_type(psr);

  if (tk->type == KW_VIOD)
    return get_void_type(psr);

  if (tk->type == KW_UNSIGNED)
    return get_unsigned_type(psr);

  if (tk->type == KW_SIGNED)
    return get_signed_type(psr);

  type = get_int_basic(psr);
  ct = new_ctype(type, 0, NULL, 0);

  return ct;
}


int
p_space(int num)
{
  for (; num > 0; num--)
    pf(" ");
}


int
ctype_print(struct ctype *ct)
{
  char *tstr;
  int i;


  if (ct->is_extern)
    pf("extern ");

  tstr = ctype_tostr(ct->type);

  if (ct->type == CT_STRUCT)
    pf("struct %s ", ct->struct_name);
  else
    pf("%s ", tstr);

  for (i = 0; i < ct->pdepth; i++)
    pf("*");
}


int
cvar_print(struct cvar *cv, int offset)
{
  p_space(offset);
  pf("%s\t-> ", cv->name);
  ctype_print(cv->type);
  pf("\n");

  return 1;
}


int
varlist_print(struct linktbl *vl, int offset)
{
  struct tblnode *n;;

  n = vl->chain;
  for (; n; n = n->next)
    cvar_print((struct cvar *) n->value, offset);

  return 1;
}


int
struct_print(struct cstruct *cs)
{
  pf("STRUCT %s:\n", cs->name);
  return varlist_print(cs->fields, 2);
}


int
structlist_print(struct linktbl *sl)
{
  struct tblnode *n;

  n = sl->chain;
  for (; n; n = n->next)
    struct_print((struct cstruct *) n->value);

  return 1;
}


int
fill_ct_pdepth(struct parser *psr, struct ctype *ct)
{
  struct token *tk;
  int pdepth;

  pdepth = 0;
  for (; psr->tk->type == TK_ASTERISK; nexttoken_notend(psr))
    pdepth++;

  tk = psr->tk;
  if (tk->type != TK_IDENT)
    exit_with("%s:%d:[PARSER]Missing identifer after type\n",
        tk->fname, tk->line);

  ct->pdepth = pdepth;

  return 1;
}


int
get_varlist(struct parser *psr, struct linktbl *vars, struct ctype *ct)
{
  struct token *tk;
  struct ctype *nct;
  struct cvar *cv;
  int i;
  char *name;

  fill_ct_pdepth(psr, ct);

  name = (char *) psr->tk->value;
  cv = new_cvar(name, ct);

  i = linktbl_put(vars, name, (void *) cv);
  if (!i)
    exit_with("%s:%d:[PARSER]<%s> has been defined\n",
        tk->fname, tk->line, name);

  nexttoken_notend(psr);
  tk = psr->tk;

  nexttoken(psr);

  if (tk->type == TK_SEMICOLON)
    return 1;

  if (tk->type != TK_COMMA)
    exit_with("%s:%d:[PARSER]Missing ';' after variables\n",
        tk->fname, tk->line);

  nct = ctype_copy(ct);
  return get_varlist(psr, vars, nct);
}


int
get_parameters(struct parser *psr, struct linktbl *params)
{
  struct token *tk;
  struct ctype *ct;
  struct cvar *cv;
  char *name;
  int i;

  ct = get_basic_type(psr);
  fill_ct_pdepth(psr, ct);

  name = (char *) psr->tk->value;
  cv = new_cvar(name, ct);

  i = linktbl_put(params, name, (void *) cv);
  if (!i)
    exit_with("%s:%d:[PARSER]<%s> has been defined\n",
        tk->fname, tk->line, name);

  nexttoken_notend(psr);
  tk = psr->tk;

  nexttoken_notend(psr);

  if (tk->type == TK_CLOSEPA)
    return 1;

  if (tk->type == TK_SEMICOLON)
    exit_with("%s:%d:[PARSER]Use ',' to split parameters\n",
        tk->fname, tk->line);

  if (tk->type != TK_COMMA)
    exit_with("%s:%d:[PARSER]Missing ')' after parameters\n",
        tk->fname, tk->line);

  return get_parameters(psr, params);
}


int
get_function_body(struct parser *psr, struct cfunc *fn)
{
  for (; psr->tk->type != TK_END; nexttoken_notend(psr));

  nexttoken_notend(psr);
  return 1;
}


int
get_function(struct parser *psr, struct ctype *ret)
{
  struct token *tk;
  struct cfunc *fn;
  struct linktbl *params;
  struct tblnode *n;
  char *name;

  fill_ct_pdepth(psr, ret);

  name = (char *) psr->tk->value;

  nexttoken_notend(psr);
  nexttoken_notend(psr);

  params = new_linktbl();
  get_parameters(psr, params);

  n = linktbl_get(psr->funcs, name);
  //check function re-definition

  fn = new_cfunc(name, ret, params);
  linktbl_put(psr->funcs, name, (void *) fn);

  tk = psr->tk;
  nexttoken_notend(psr);

  if (tk->type == TK_SEMICOLON)
    return 1;

  if (tk->type != TK_BEGIN)
    return get_function_body(psr, fn);

  exit_with("%s:%d:[PARSER]Missing '{' after parameters\n",
      tk->fname, tk->line);

  return 1;
}


int
is_function_or_var(struct parser *psr)
{
  struct token *tk;

  for (tk = psr->tk; tk && tk->type == TK_ASTERISK; tk = tk->next);

  if (tk && tk->next && tk->next->type == TK_OPENPA)
    return 1;
  else
    return 0;
}


int
is_struct_definition(struct parser *psr)
{
  struct token *tk;

  tk = psr->tk;
  if (tk->type == KW_STRUCT && tk->next && tk->next->next &&
      tk->next->next->type == TK_BEGIN)
    return 1;
  else
    return 0;
}


int
get_struct_fields(struct parser *psr, struct linktbl *fields)
{
  struct token *tk;
  struct ctype *ct;

  ct = get_basic_type(psr);
  get_varlist(psr, fields, ct);

  if (psr->tk->type != TK_END)
    return get_struct_fields(psr, fields);
  else
    return 1;
}


int
get_struct_definition(struct parser *psr)
{
  struct linktbl *fields;
  struct token *tk;
  struct cstruct *cs;
  char *name;

  tk = nexttoken_notend(psr);
  name = (char *) tk->value;

  nexttoken_notend(psr);
  nexttoken_notend(psr);

  fields = new_linktbl();
  get_struct_fields(psr, fields);

  tk = psr->tk;
  nexttoken_notend(psr);

  if (psr->tk->type != TK_SEMICOLON)
    exit_with("%s:%d:[PARSER]Missing ';' after struct definition\n",
        tk->fname, tk->line);

  nexttoken_notend(psr);

  cs = new_cstruct(name, fields);

  linktbl_put(psr->sdefs, name, (void *) cs);

  return 1;
}


int
get_variables_or_func(struct parser *psr)
{
  struct token *tk;
  struct ctype *ct;
  int is_extern;

  tk = psr->tk;
  if (tk->type == KW_EXTERN)
    is_extern = 1;
  else
    is_extern = 0;

  if (is_extern)
    tk = nexttoken_notend(psr);

  ct = get_basic_type(psr);
  ct->is_extern = is_extern;

  if (is_function_or_var(psr))
    return get_function(psr, ct);
  else
    return get_varlist(psr, psr->gvars, ct);
}


struct parser *
new_parser(struct token *tks)
{
  struct parser *p;

  p = malloc(sizeof(struct parser));
  if (!p)
    exit_with("Failed alloc memory for parser\n");

  p->funcs = new_linktbl();
  p->gvars = new_linktbl();
  p->sdefs = new_linktbl();

  p->tk = tks;

  return p;
}


int
parse_recur(struct parser *psr)
{
  struct token *tk;
  if (!psr->tk)
    return 1;

  if (is_struct_definition(psr))
    get_struct_definition(psr);
  else
    get_variables_or_func(psr);

  return parse_recur(psr);
}


int
parse(struct token *tks)
{
  struct parser *psr;

  psr = new_parser(tks);

  parse_recur(psr);

  pf("Global variables:\n");
  varlist_print(psr->gvars, 2);
  structlist_print(psr->sdefs);

  return 1;
}


