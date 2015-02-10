#include <stdio.h>
#include <stdlib.h>
#include "language.h"
#include "log.h"

int lang_eval_returned;
int lang_eval_tok;
char* lang_eval_parse_string(FILE* lang_in);
int   lang_eval_doIf_int(FILE* lang_in,char* filename);
void  lang_eval_doIf_void(FILE* lang_in,char* filename);
char* lang_eval_doIf_string(FILE* lang_in,char* filename);
int   lang_eval_doFor_int(FILE* lang_in,char* filename);
void  lang_eval_doFor_void(FILE* lang_in,char* filename);
char* lang_eval_doFor_string(FILE* lang_in,char* filename);
int   lang_eval_doWhile_int(FILE* lang_in,char* filename);
void  lang_eval_doWhile_void(FILE* lang_in,char* filename);
char* lang_eval_doWhile_string(FILE* lang_in,char* filename);

//This should be called after the LANG_CALL token is eaten.
char* lang_eval_doCallPreliminary(FILE* lang_in){
  lang_skipWhitespace(lang_in);
  char* name;
  name = lang_getVarName(lang_in);
  char exit;
  exit = 0;
  int tok;
  tok = lang_findToken(lang_in);
  if (tok == LANG_OBRACKET) tok = lang_findToken(lang_in);
  while (exit == 0){
    //debug_var("Token",tok);
    if (tok == LANG_VAR){
      char* varname;
      int varid;
      int vartype;
      varname = lang_getVarName(lang_in);
      varid = lang_getVarId(varname);
      vartype = lang_getVarType(varid);
      if (vartype == LANG_INT){
        int varint;
        varint = lang_getInt(varname);
        lang_addArgument_int(varint);
      }
      else if (vartype == LANG_STRING){
        char* varstring;
        varstring = lang_getString(varname);
        lang_addArgument_string(varstring);
      }
    }
    else if (tok == LANG_QUOTE){
      char* strconst;
      strconst = lang_eval_parse_string(lang_in);
      lang_addArgument_string(strconst);
    }
    else if (tok == LANG_NUMBER){
      int num;
      fscanf(lang_in,"%d",&num);
      lang_skipWhitespace(lang_in);
      lang_addArgument_int(num);
      //debug_var("Number",num);
    }
    else if (tok == LANG_CBRACKET){
      exit = 1;
      break;
    }
    else{
      printf("Unexpected token %d while parsing LANG_CALL\n",tok);
      lang_error=1;
      return "";
    }
    tok = lang_findToken(lang_in);
  }
  return name;
}
int lang_eval_getNumber(FILE* lang_in){
  int tok;
  int ret;
  ret = -1;
  tok = lang_findToken(lang_in);
  if (tok == LANG_NUMBER){
    fscanf(lang_in,"%d",&ret);
  }
  else if (tok == LANG_VAR){
    char* name;
    name = lang_getVarName(lang_in);
    ret = lang_getInt(name);
  }
  else{
    printf("Unexpected token while getting number: %d\n",tok);
    lang_error = 1;
  }
  return ret;
}
char* lang_eval_doCall_string(FILE* lang_in){
  char* ret;
  char* name;
  name = lang_eval_doCallPreliminary(lang_in);
  ret = lang_callFunction_string(name);
  return ret;
}
int lang_eval_doCall_int(FILE* lang_in){
  int ret;
  char* name;
  name = lang_eval_doCallPreliminary(lang_in);
  ret = lang_callFunction_int(name);
  return ret;
}
void lang_eval_doCall_void(FILE* lang_in){
  char* name;
  name = lang_eval_doCallPreliminary(lang_in);
  lang_callFunction_void(name);
}
int lang_eval_getVar_int(FILE* lang_in){
  int tok;
  tok = lang_findToken(lang_in);
  int ret;
  ret = 0;
  if (tok == LANG_NUMBER){
    //Get the number
    fscanf(lang_in,"%d",&ret);
  }
  else if (tok == LANG_VAR){
    //Get the variable
    char* name;
    name = lang_getVarName(lang_in);
    //Can not promote anything to an int, so do not try to get other variable types
    ret = lang_getInt(name);
  }
  else if (tok == LANG_CALL){
    ret = lang_eval_doCall_int(lang_in);
  }
  else if (tok == LANG_OBRACKET){
    ret = lang_eval_stmt_int(lang_in);
  }
  else{
    printf("Invalid token during getVar: %d\n",tok);
    lang_error = 1;
  }
  tok = lang_findToken(lang_in);
  if (tok != LANG_PLUS && tok != LANG_MINUS && tok != LANG_TIMES && tok != LANG_DIVIDE && tok != LANG_NOTHING && tok != LANG_COMMA && tok != LANG_CBRACKET && tok != LANG_GT && tok != LANG_LT && tok != LANG_EQ && tok != LANG_NEQ && tok != LANG_GTE && tok != LANG_LTE && tok != LANG_AND && tok != LANG_OR){
    printf("Unexpected token while getting variable: %d\n",tok);
  }
  else if (tok == LANG_TIMES){
    ret = ret * lang_eval_getVar_int(lang_in);
  }
  else if (tok == LANG_DIVIDE){
    ret = ret / lang_eval_getVar_int(lang_in);
  }
  else if (tok == LANG_CBRACKET){
    ungetc(')',lang_in);
  }
  else if (tok == LANG_NOTHING){
    ungetc('\n',lang_in);
  }
  else if (tok == LANG_GT){
    ret = ret > lang_eval_getVar_int(lang_in);
  }
  else if (tok == LANG_LT){
    ret = ret < lang_eval_getVar_int(lang_in);
  }
  else if (tok == LANG_EQ){
    ret = ret == lang_eval_getVar_int(lang_in);
  }
  else if (tok == LANG_NEQ){
    ret = ret != lang_eval_getVar_int(lang_in);
  }
  else if (tok == LANG_GTE){
    ret = ret >= lang_eval_getVar_int(lang_in);
  }
  else if (tok == LANG_LTE){
    ret = ret <= lang_eval_getVar_int(lang_in);
  }
  else if (tok == LANG_AND){
    ungetc('&',lang_in);
  }
  else if (tok == LANG_OR){
    ungetc('|',lang_in);
  }
  return ret;
} 
int lang_eval_stmt_int(FILE* lang_in){
  int ret;
  int tok;
  lang_eval_tok=-1;
  ret = lang_eval_getVar_int(lang_in);
  if (lang_eval_tok==-1){
    tok = lang_findToken(lang_in);
  }
  else{
    tok = lang_eval_tok;
    lang_eval_tok = -1;
  }
  if (tok != LANG_PLUS && tok != LANG_MINUS && tok != LANG_NOTHING && tok != LANG_COMMA && tok != LANG_CBRACKET && tok != LANG_AND && tok != LANG_OR){
    printf("Unexpected token while evaluating int-stmt: %d\n",tok);
  }

  else if (tok == LANG_PLUS){
    fgetc(lang_in);
    ret = ret + lang_eval_stmt_int(lang_in);
  }
  else if (tok == LANG_MINUS){
    fgetc(lang_in);
    ret = ret - lang_eval_stmt_int(lang_in);
  }
  else if (tok == LANG_AND){
    ret = ret & lang_eval_stmt_int(lang_in);
  }
  else if (tok == LANG_OR){
    ret = ret | lang_eval_stmt_int(lang_in);
  }
  else if (tok == LANG_NOTHING){
    ungetc('\n',lang_in);
  }
  return ret;
}
char* lang_eval_parse_string(FILE* lang_in){
    char* str;
    str = malloc(LANG_DEFAULT_STRLEN);
    char c;
    c = fgetc(lang_in);
    int i;
    i = 0;
    str[0] = 0;
    if (c == '"') return str;
    while (c!='"' && i < LANG_DEFAULT_STRLEN-1){
      if (c=='\\'){
        c=fgetc(lang_in);
        if (c=='n'){
          c=10;
        }
        else{
          printf("Unknown special code \%c\n",c);
          lang_error=1;
          free(str);
          return NULL;
        }
      }
      if (c==EOF){
        printf("Unexpected EOF while parsing string\n");
        free(str);
        lang_error=1;
        return NULL;
      }
      str[i]=c;
      ++i;
      c=fgetc(lang_in);
    }
    if (i==79 && c!='"'){
      printf("String was too long\n");
      free(str);
      lang_error=1;
      return NULL;
    }
    return str;
}
char* lang_eval_stmt_string(FILE* lang_in){
  int tok;
  tok = lang_findToken(lang_in);
  if (tok == LANG_QUOTE){
    return lang_eval_parse_string(lang_in);
  }
  else if (tok == LANG_VAR){
    return lang_getString(lang_getVarName(lang_in));
  }
  else if (tok == LANG_CALL){
    return lang_eval_doCall_string(lang_in);
  }
  else {
    printf("Unexpected token: %d\n",tok);
    return NULL;
  }
  return NULL;
}
char lang_eval_stmt(FILE* lang_in){
  char* name;
  name = lang_getVarName(lang_in);
  int tok;
  tok = lang_findToken(lang_in);
  if (tok != LANG_ASSIGN){
    printf("Expected '='\n");
    lang_error=1;
    return -1;
  }
  int id;
  id = lang_getVarId(name);
  if (id !=-1){
    if (lang_getVarType(id) == LANG_INT){
      lang_setVar_int(id,lang_eval_stmt_int(lang_in));
    }
    else if (lang_getVarType(id) == LANG_STRING){
      lang_setVar_string(id,lang_eval_stmt_string(lang_in));
    }
    else {
      printf("Unknown type\n");
      lang_error=1;
    }
  }
  else{
    printf("Could not find variable: %s\n",name);
    lang_error=1;
  }
}
char* lang_eval_function_string(char* filename, int id){
  lang_eval_returned = 0;
  FILE* func_in;
  func_in = fopen(filename,"r");
  if (func_in==NULL){
    printf("Filename is invalid\n");
    return NULL;
  }
  int line;
  for (line=0;line<id;++line){
    lang_skipLine(func_in);
  }  
  //Should now be on the correct line
  char c;
  c = fgetc(func_in);
  while (c!='{'){
    c = fgetc(func_in);
  }
  lang_skipWhitespace(func_in);
  int tok; 
  tok = lang_findToken(func_in);
  while (tok !=-1 && tok!=LANG_END_FUNC){
    lang_eval_returned=0;
    if (tok == LANG_INT) lang_parseInt(func_in);
    else if (tok == LANG_STRING) lang_parseString(func_in);
    else if (tok == LANG_VAR) lang_eval_stmt(func_in);
    else if (tok == LANG_CALL) lang_eval_doCall_void(func_in);
    else if (tok == LANG_IF){
      char* ret;
      ret = lang_eval_doIf_string(func_in,filename);
      if (lang_eval_returned==1){
        return ret;
      }
    }
    else if (tok == LANG_WHILE){
      char* ret;
      ret = lang_eval_doWhile_string(func_in,filename);
      if (lang_eval_returned == 1){
        return ret;
      }
    }
    else if (tok == LANG_FOR){
      char* ret;
      ret = lang_eval_doFor_string(func_in,filename);
      if (lang_eval_returned == 1){
        return ret;
      }
    }
    else if (tok == LANG_RETURN){
      lang_eval_returned = 1;
      char* ret;
      ret=lang_eval_stmt_string(func_in);
      return ret;
    }
    if (tok != LANG_NOTHING) lang_skipLine(func_in);
    tok = lang_findToken(func_in);
  }
  return NULL;
}

int lang_eval_function_int(char* filename, int id){
  lang_eval_returned = 0;
  FILE* func_in;
  func_in = fopen(filename,"r");
  if (func_in==NULL){
    printf("Filename is invalid\n");
    return 0;
  }
  int line;
  for (line=0;line<id;++line){
    lang_skipLine(func_in);
  }  
  //Should now be on the correct line
  char c;
  c = fgetc(func_in);
  while (c!='{'){
    c = fgetc(func_in);
  }
  lang_skipWhitespace(func_in);
  int tok; 
  tok = lang_findToken(func_in);
  while (tok !=-1 && tok!=LANG_END_FUNC){
    lang_eval_returned=0;
    if (tok == LANG_INT) lang_parseInt(func_in);
    else if (tok == LANG_STRING) lang_parseString(func_in);
    else if (tok == LANG_VAR) lang_eval_stmt(func_in);
    else if (tok == LANG_CALL) lang_eval_doCall_void(func_in);
    else if (tok == LANG_IF){
      int ret;
      ret = lang_eval_doIf_int(func_in,filename);
      if (lang_eval_returned==1){
        return ret;
      }
    }
    else if (tok == LANG_WHILE){
      int ret;
      ret = lang_eval_doWhile_int(func_in,filename);
      if (lang_eval_returned == 1){
        return ret;
      }
    }
    else if (tok == LANG_FOR){
      int ret;
      ret = lang_eval_doFor_int(func_in,filename);
      if (lang_eval_returned == 1){
        return ret;
      }
    }
    else if (tok == LANG_RETURN){
      lang_eval_returned = 1;
      int ret;
      ret=lang_eval_stmt_int(func_in);
      return ret;
    }
    if (tok != LANG_NOTHING) lang_skipLine(func_in);
    tok = lang_findToken(func_in);
  }
  return 0;
}
void lang_eval_function_void(char* filename, int id){
  FILE* func_in;
  func_in = fopen(filename,"r");
  if (func_in==NULL){
    printf("Filename is invalid\n");
    return;
  }
  int line;
  for (line=0;line<id;++line){
    lang_skipLine(func_in);
  }  
  //Should now be on the correct line
  char c;
  c = fgetc(func_in);
  while (c!='{'){
    c = fgetc(func_in);
  }
  lang_skipWhitespace(func_in);
  int tok; 
  tok = lang_findToken(func_in);
  while (tok !=-1 && tok!=LANG_END_FUNC){
    lang_eval_returned=0;
    if (tok == LANG_INT) lang_parseInt(func_in);
    else if (tok == LANG_STRING) lang_parseString(func_in);
    else if (tok == LANG_VAR) lang_eval_stmt(func_in);
    else if (tok == LANG_CALL) lang_eval_doCall_void(func_in);
    else if (tok == LANG_IF){
      lang_eval_doIf_void(func_in,filename);
      if (lang_eval_returned==1){
        return;
      }
    }
    else if (tok == LANG_WHILE){
      lang_eval_doWhile_void(func_in,filename);
      if (lang_eval_returned==1){ 
        return;
      }
    }
    else if (tok == LANG_FOR){
      lang_eval_doFor_void(func_in,filename);
      if (lang_eval_returned == 1){
        return;
      }
    }
    else if (tok == LANG_RETURN){
      return;
    }
    if (tok != LANG_NOTHING) lang_skipLine(func_in);
    tok = lang_findToken(func_in);
  }
  return;
}
int lang_eval_getLinePos(FILE* lang_in,char* filename){
  long pos;
  FILE* nfile;
  long i;
  int nlcount;
  nlcount = 0;
  pos = ftell(lang_in);
  nfile = fopen(filename,"r");
  for (i = 0;i <= pos; ++i){
    char c;
    c = fgetc(nfile);
    if (c == '\n'){
      ++nlcount;
    }
  }
  return nlcount;
}
void lang_eval_doIf_void(FILE* lang_in,char* filename){
  int stmt;
  char c;
  int level;
  level = 1;
  stmt = lang_eval_stmt_int(lang_in);
  if (stmt!=0){
   lang_eval_function_void(filename,lang_eval_getLinePos(lang_in,filename));
  }  
  c = fgetc(lang_in);
  while (c != '{'){
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    c = fgetc(lang_in);
  }
  while (level > 0)
  {
    c = fgetc(lang_in);
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file while parsing if\n");
    }
    if (c == '{') ++level;
    if (c == '}') --level;
  }
}
int lang_eval_doIf_int(FILE* lang_in,char* filename){
  int stmt;
  int ret;
  char c;
  int level;
  level = 1;
  ret = 0;
  stmt = lang_eval_stmt_int(lang_in);
  if (stmt!=0){
   ret = lang_eval_function_int(filename,lang_eval_getLinePos(lang_in,filename));
  }  
  c = fgetc(lang_in);
  while (c != '{'){
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    c = fgetc(lang_in);
  }
  while (level > 0)
  { 
    c = fgetc(lang_in);
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    if (c == '{') ++level;
    if (c == '}') --level;
  }

  return ret;
}
char* lang_eval_doIf_string(FILE* lang_in,char* filename){
  int stmt;
  char* ret;
  char c;
  int level;
  level = 1;
  ret = 0;
  stmt = lang_eval_stmt_int(lang_in);
  if (stmt!=0){
   ret = lang_eval_function_string(filename,lang_eval_getLinePos(lang_in,filename));
  }  
  c = fgetc(lang_in);
  while (c != '{'){
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    c = fgetc(lang_in);
  }
  while (level > 0)
  { 
    c = fgetc(lang_in);
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    if (c == '{') ++level;
    if (c == '}') --level;
  }

  return ret;
}
int lang_eval_doFor_int(FILE* lang_in,char* filename){
  char* name;
  int start;
  int end;
  char c;
  int level;
  level = 1;
  name = lang_getVarName(lang_in);
  fscanf(lang_in,"%d",&start);
  fscanf(lang_in,"%d",&end);
  if (end > start)
  {
    int varid;
    int line;
    int i;
    lang__defVar(name,LANG_INT);
    varid = lang_getVarId(name);
    line = lang_eval_getLinePos(lang_in,filename);
    for (i = start; i < end; i++)
    {
      int ret;
      lang_setVar_int(varid,i);
      ret = lang_eval_function_int(filename,line);
      if (lang_eval_returned==1){
        return ret;
      }
    }
  }
  if (end < start)
  {
    int varid;
    int line;
    int i;
    lang__defVar(name,LANG_INT);
    varid = lang_getVarId(name);
    line = lang_eval_getLinePos(lang_in,filename);
    for (i = start; i > end; i--)
    {
      int ret;
      lang_setVar_int(varid,i);
      ret = lang_eval_function_int(filename,line);
      if (lang_eval_returned==1){
        return ret;
      }
    }
  }
  c = fgetc(lang_in);
  while (c != '{'){
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing for\n");
    }
    c = fgetc(lang_in);
  }
  while (level > 0)
  { 
    c = fgetc(lang_in);
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing for\n");
    }
    if (c == '{') ++level;
    if (c == '}') --level;
  }
}
void lang_eval_doFor_void(FILE* lang_in,char* filename){
  char* name;
  int start;
  int end;
  char c;
  int level;
  level = 1;
  name = lang_getVarName(lang_in);
  fscanf(lang_in,"%d",&start);
  fscanf(lang_in,"%d",&end);
  if (end > start)
  {
    int varid;
    int line;
    int i;
    lang__defVar(name,LANG_INT);
    varid = lang_getVarId(name);
    line = lang_eval_getLinePos(lang_in,filename);
    for (i = start; i < end; i++)
    {
      lang_setVar_int(varid,i);
      lang_eval_function_void(filename,line);
      if (lang_eval_returned==1){
        return;
      }
    }
  }
  if (end < start)
  {
    int varid;
    int line;
    int i;
    lang__defVar(name,LANG_INT);
    varid = lang_getVarId(name);
    line = lang_eval_getLinePos(lang_in,filename);
    for (i = start; i > end; i--)
    {
      int ret;
      lang_setVar_int(varid,i);
      lang_eval_function_void(filename,line);
      if (lang_eval_returned==1){
        return;
      }
    }
  }
  c = fgetc(lang_in);
  while (c != '{'){
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing for\n");
    }
    c = fgetc(lang_in);
  }
  while (level > 0)
  { 
    c = fgetc(lang_in);
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing for\n");
    }
    if (c == '{') ++level;
    if (c == '}') --level;
  }
}
char* lang_eval_doFor_string(FILE* lang_in,char* filename){
  char* name;
  int start;
  int end;
  char c;
  int level;
  level = 1;
  name = lang_getVarName(lang_in);
  fscanf(lang_in,"%d",&start);
  fscanf(lang_in,"%d",&end);
  if (end > start)
  {
    int varid;
    int line;
    int i;
    lang__defVar(name,LANG_INT);
    varid = lang_getVarId(name);
    line = lang_eval_getLinePos(lang_in,filename);
    for (i = start; i < end; i++)
    {
      char* ret;
      lang_setVar_int(varid,i);
      ret = lang_eval_function_string(filename,line);
      if (lang_eval_returned==1){
        return ret;
      }
    }
  }
  if (end < start)
  {
    int varid;
    int line;
    int i;
    lang__defVar(name,LANG_INT);
    varid = lang_getVarId(name);
    line = lang_eval_getLinePos(lang_in,filename);
    for (i = start; i > end; i--)
    {
      char* ret;
      lang_setVar_int(varid,i);
      ret = lang_eval_function_string(filename,line);
      if (lang_eval_returned==1){
        return ret;
      }
    }
  }
  c = fgetc(lang_in);
  while (c != '{'){
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing for\n");
    }
    c = fgetc(lang_in);
  }
  while (level > 0)
  { 
    c = fgetc(lang_in);
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing for\n");
    }
    if (c == '{') ++level;
    if (c == '}') --level;
  }
}
int lang_eval_doWhile_int(FILE* lang_in,char* filename){
  int stmt;
  int ret;
  char c;
  int level;
  fpos_t position;
  level = 1;
  ret = 0;
  fgetpos(lang_in,&position);
  stmt = lang_eval_stmt_int(lang_in);
  while (stmt != 0){
    ret = lang_eval_function_int(filename,lang_eval_getLinePos(lang_in,filename));
    if (lang_eval_returned==1){
      return ret;
    }
    fsetpos(lang_in,&position);
    stmt = lang_eval_stmt_int(lang_in);
  }
  c = fgetc(lang_in);
  while (c != '{'){
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    c = fgetc(lang_in);
  }
  while (level > 0)
  { 
    c = fgetc(lang_in);
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    if (c == '{') ++level;
    if (c == '}') --level;
  }
  return ret;
}
char* lang_eval_doWhile_string(FILE* lang_in,char* filename){
  int stmt;
  char* ret;
  char c;
  int level;
  fpos_t position;
  level = 1;
  ret = 0;
  fgetpos(lang_in,&position);
  stmt = lang_eval_stmt_int(lang_in);
  while (stmt != 0){
    ret = lang_eval_function_string(filename,lang_eval_getLinePos(lang_in,filename));
    if (lang_eval_returned==1){
      return ret;
    }
    fsetpos(lang_in,&position);
    stmt = lang_eval_stmt_int(lang_in);
  }
  c = fgetc(lang_in);
  while (c != '{'){
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    c = fgetc(lang_in);
  }
  while (level > 0)
  { 
    c = fgetc(lang_in);
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    if (c == '{') ++level;
    if (c == '}') --level;
  }
  return ret;
}
void lang_eval_doWhile_void(FILE* lang_in,char* filename){
  int stmt;
  char c;
  int level;
  fpos_t position;
  level = 1;
  fgetpos(lang_in,&position);
  stmt = lang_eval_stmt_int(lang_in);
  while (stmt != 0){
    lang_eval_function_void(filename,lang_eval_getLinePos(lang_in,filename));
    if (lang_eval_returned==1){
      return;
    }
    fsetpos(lang_in,&position);
    stmt = lang_eval_stmt_int(lang_in);
  }
  c = fgetc(lang_in);
  while (c != '{'){
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    c = fgetc(lang_in);
  }
  while (level > 0)
  { 
    c = fgetc(lang_in);
    if (c == EOF){
      lang_error = 1;
      printf("Unexpected (end-of-file) while parsing if\n");
    }
    if (c == '{') ++level;
    if (c == '}') --level;
  }
  return;
}
