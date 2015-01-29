#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "language.h"
#include "log.h"
#include "eval.h"

char* lang_var_name[LANG_NUM_VARS];
int lang_var_type[LANG_NUM_VARS];
int lang_var_int[LANG_NUM_VARS];
char* lang_var_string[LANG_NUM_VARS];
char lang_nextArg;
int lang_nextVarId;
int lang_oVarId;
int lang_lineNum;
char* lang_filename;
int DEBUG = 0;


//Defined as extern in language.h
int lang_error;
int lang_getVarId(char* name){
  int id = -1;
  int ii;
  for (ii=lang_nextVarId;ii>=0;--ii){
    if (strcmp(lang_var_name[ii],name)==0){
      id = ii;
      break;
    }
  }
  return id;
}

int lang_getVarType(int id){
  return lang_var_type[id];
}

char lang_setVar_int(int id, int val){
  if (id > lang_nextVarId || id < 0){
    lang_error = 1;
    return -1;
  }
  lang_var_int[id] = val;
  return 0;
}
char lang_setVar_string(int id, char* val){
  if (id > lang_nextVarId || id < 0){
    lang_error = 1;
    return -1;
  }
  lang_var_string[id] = val;
  return 0;
}

int lang_nextVar(){
  ++lang_nextVarId;
  if (lang_nextVarId < LANG_NUM_VARS){
    return lang_nextVarId;
  }
  return -1;
}

void lang_skipWhitespace(FILE* lang_in){
  //This skips all whitespace except for new lines
  char exit;
  exit = 0;
  char c;
  c = fgetc(lang_in);
  while (c == ' ' || c == '\t'){
    c = fgetc(lang_in);
  }
  ungetc(c,lang_in);
}

void lang_skipLine(FILE* lang_in){
  char c;
  c = fgetc(lang_in);
  while (c != '\n' && c != EOF){
    c = fgetc(lang_in);
  }
}

int lang_findToken(FILE* lang_in){
  //Skip any whitespace
  lang_skipWhitespace(lang_in);
  char c;
  c = fgetc(lang_in);
  if (c == '#') return LANG_COMMENT;
  else if (c == '\n') return LANG_NOTHING;
  else if (c == '$') return LANG_VAR;
  else if (c == '}') return LANG_END_FUNC;
  else if (c == ',') return LANG_COMMA;
  else if (c == '*') return LANG_TIMES;
  else if (c == '/') return LANG_DIVIDE;
  else if (c == '(') return LANG_OBRACKET;
  else if (c == ')') return LANG_CBRACKET;
  else if (c == '"') return LANG_QUOTE;
  else if (c == '@') return LANG_CALL;
  else if (c == ';') return LANG_SEMICOLN;
  else if (c == '&') return LANG_AND;
  else if (c == '|') return LANG_OR;
  else if (c == '+'){
    ungetc(c,lang_in);
    return LANG_PLUS;
  }
  else if (c == '-'){
    ungetc(c,lang_in);
    return LANG_MINUS;
  }
  else if (c >= '0' && c <='9'){
    ungetc(c,lang_in);
    return LANG_NUMBER;
  }
  else if (c == EOF){
    //printf("Reached end-of-file\n");
    return -1;
  }
  ungetc(c,lang_in);
  char string[80];
  fscanf(lang_in,"%79s",string);
  //Check for multichar tokens
  if (strcmp(string,"int")==0) return LANG_INT;
  else if (strcmp(string,"string")==0) return LANG_STRING;
  else if (strcmp(string,"function")==0) return LANG_FUNC;
  else if (strcmp(string,"end")==0) return LANG_END_FUNC;
  else if (strcmp(string,"return")==0) return LANG_RETURN;
  else if (strcmp(string,"call")==0) return LANG_CALL;
  else if (strcmp(string,"if")==0) return LANG_IF;
  else if (strcmp(string,"while")==0) return LANG_WHILE;
  else if (strcmp(string,"=")==0) return LANG_ASSIGN;
  else if (strcmp(string,"==")==0) return LANG_EQ;
  else if (strcmp(string,"!=")==0) return LANG_NEQ;
  else if (strcmp(string,"!")==0) return LANG_NOT;
  else if (strcmp(string,">=")==0) return LANG_GTE;
  else if (strcmp(string,"<=")==0) return LANG_LTE;
  else if (strcmp(string,"<")==0) return LANG_LT;
  else if (strcmp(string,">")==0) return LANG_GT;
  debug_str("Could not find a matching token for",string);
  return -1;
}

char* lang_getVarName(FILE* lang_in){
  lang_skipWhitespace(lang_in);
  char* name;
  name = (char*)malloc(80);
  char c;
  c = fgetc(lang_in);
  int i;
  i=-1;
  while(((c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_') && i<79)
  {
    name[++i]=c;
    c = fgetc(lang_in);
  }
  ungetc(c,lang_in);
  name[++i]=0;
  return name;
}

char lang__defVar(char* name,int type){

  int id;
  id = lang_nextVar();
  if (id!=-1)
  {
    lang_var_name[id]=name;
    lang_var_type[id]=type;
    debug_str("Defined variable",name);
    return 0;
  }
  puts("Could not allocate another variable.\nIncrease LANG_NUM_VARS");
  return -1;
}

char lang_defVar(FILE* lang_in,int type){
  char* name;
  name = lang_getVarName(lang_in);
  return lang__defVar(name,type);
}

char lang_parseFunction(FILE* lang_in){
  lang_defVar(lang_in,LANG_FUNC);
  //insert the line number into int and skip to }, counting newlines
  lang_var_int[lang_nextVarId]=lang_lineNum;
  debug_var("  Function is on line",lang_lineNum+1);
  int level;
  level = 1;
  lang_skipWhitespace(lang_in);
  char c = fgetc(lang_in);
  if (c == '{') c = fgetc(lang_in);
  while (level > 0){
    if (c=='\n'){
     ++lang_lineNum;
    }
    else if (c=='{') ++level;
    else if (c=='}') --level;
    c = fgetc(lang_in);
  }
  ungetc(c,lang_in);
  return 0;
}

char lang_parseInt(FILE* lang_in){
  lang_defVar(lang_in,LANG_INT);
  int tok = lang_findToken(lang_in);
  if (tok==LANG_NOTHING){
    ungetc('\n',lang_in);
    return 0;
  }
  else if (tok!=LANG_ASSIGN){
    printf("Expected '=' or '\\n' not %d\n",tok);
    return -1;
  }
  lang_var_int[lang_nextVarId]=lang_eval_stmt_int(lang_in);
  return 0;
}

char lang_parseString(FILE* lang_in){
  lang_defVar(lang_in,LANG_STRING);
  int tok = lang_findToken(lang_in);
  if (tok==LANG_NOTHING){
    ungetc('\n',lang_in);
    return 0;
  }
  else if (tok!=LANG_ASSIGN){
    printf("Expected '=' or '\\n' not %d\n",tok);
    return -1;
  }
  lang_var_string[lang_nextVarId]=lang_eval_stmt_string(lang_in);
  return 0;
}

char lang_skipFunction(FILE* lang_in){
  int level;
  char c;
  level = 1;
  c = fgetc(lang_in);
  while (c!='{' && c!=EOF){
    c = fgetc(lang_in);
  }
  c = fgetc(lang_in);
  while (level > 0){
    if (c == '{') ++level;
    else if (c == '}') --level;
    c = fgetc(lang_in);
  }
  ungetc(c,lang_in);
  return 0;
}

char lang_firstParse(FILE* lang_in){
  int tok;
  tok = lang_findToken(lang_in);
  while ( tok != -1)
  {
    //debug_var("Found Token",tok);
    
    if (tok == LANG_FUNC) lang_parseFunction(lang_in);
    else if ( tok == LANG_VAR     || tok == LANG_RETURN    ||
              tok == LANG_CALL    || tok == LANG_END_FUNC  ||
              tok == LANG_NUMBER  || tok == LANG_IF        ||
              tok == LANG_WHILE){
      printf("Found invalid token: %d\n",tok);
      return -1;
    }
    if (tok != LANG_NOTHING) lang_skipLine(lang_in);
    tok = lang_findToken(lang_in);
    lang_lineNum++;
  }
  return 1;
}

char lang_secondParse(FILE* lang_in){
  int tok;
  tok = lang_findToken(lang_in);
  while (tok != -1 )
  {
    if (tok == LANG_FUNC) lang_skipFunction(lang_in);
    else if (tok == LANG_INT) lang_parseInt(lang_in);
    else if (tok == LANG_STRING) lang_parseString(lang_in);
    else if ( tok == LANG_VAR         || tok == LANG_RETURN     ||
              tok == LANG_CALL        || tok == LANG_END_FUNC   ||
              tok == LANG_IF          || tok == LANG_WHILE){
      printf("Found invalid token during second parse: %d\n",tok);
      return -1;
    }
    if (tok != LANG_NOTHING) lang_skipLine(lang_in);
    tok = lang_findToken(lang_in);
   //printf("tok = %d\n",tok);
  }
}

char lang_loadFile(char* filename){
  lang_nextVarId=-1;
  lang_lineNum=0;
  lang_nextArg='A';
  lang_error=0;
  lang_oVarId = -1;
  lang_filename=filename;
  FILE* lang_in;
  lang_in = fopen(filename,"r");
  if (lang_firstParse(lang_in)==-1){
    return -1;
  }
  rewind(lang_in);
  lang_secondParse(lang_in);
  fclose(lang_in);
}

char lang_addArgument_int(int arg){
  if (lang_nextArg=='A'){
    lang_oVarId = lang_nextVarId;
  }
  if (lang_nextArg>'Z'){
    puts("Can not bind more than 26 Arguments!\n");
    return -1;
  }

  char* str;
  str = malloc(5);
  str[0]=0;
  strcat(str,"arg");
  str[3]=lang_nextArg;
  str[4]=0;
  lang__defVar(str,LANG_INT);
  lang_var_int[lang_nextVarId]=arg;
  ++lang_nextArg;

  return 0;
}

char lang_addArgument_string(char* arg){
  if (lang_nextArg=='A'){
    lang_oVarId = lang_nextVarId;
  }
  if (lang_nextArg>'Z'){
    puts("Can not bind more than 26 Arguments!\n");
    return -1;
  }
  char* str;
  str = malloc(5);
  str[0]=0;
  strcat(str,"arg");
  str[3]=lang_nextArg;
  str[4]=0;
  lang__defVar(str,LANG_STRING);
  lang_var_string[lang_nextVarId]=arg;
  ++lang_nextArg;
  return 0;
}

int lang_getInt(char* variableName){
  int ii;
  for (ii=lang_nextVarId;ii>=0;--ii){
    if (strcmp(lang_var_name[ii],variableName)==0){
      if (lang_var_type[ii]==LANG_INT || lang_var_type[ii]==LANG_FUNC){
        return lang_var_int[ii];
      }else{
        printf("Found variable, but was of wrong type\n");
        lang_error=1;
        return 0;
      }
    }
  }
  printf("Did not find Variable %s\n",variableName);
  lang_error=1;
  return 0;
}

char* lang_getString(char* variableName){
    int ii;
    for (ii=lang_nextVarId;ii>=0;--ii){
    if (strcmp(lang_var_name[ii],variableName)==0){
      if (lang_var_type[ii]==LANG_STRING){
        return lang_var_string[ii];
      }else{
        printf("Found variable, but was of wrong type\n");
        lang_error=1;
        return NULL;
      }
    }
  }
  printf("Did not find Variable %s|n",variableName);
  lang_error=1;
  return NULL;
}
void lang_freeVariables(int oVarId){
  int i;
  for (i = oVarId; i > lang_nextVarId; i--){
    free(lang_var_name[i]);
    if (lang_var_type[i]==LANG_STRING){
      free(lang_var_string[i]);
    }
  }
}
int lang_callFunction_int(char* functionName){
  int line;
  int ret = 0;
  int local_oVarId;
  if (lang_oVarId == -1)
  {
    lang_oVarId = lang_nextVarId;
  }
  local_oVarId = lang_oVarId;
  lang_oVarId = -1;
  lang_nextArg = 'A';
  if (strcmp(functionName,"puts")==0){
    printf("%s",lang_getString("argA"));
  }
  else if (strcmp(functionName,"puti")==0){
    printf("%d",lang_getInt("argA"));
  }
  else{
    line = lang_getInt(functionName);
    if (lang_error!=1){
      ret = lang_eval_function_int(lang_filename,line);
    }
    else{
      printf("Could not find function: %s\n",functionName);
      lang_error=1;
    }
  }
  lang_freeVariables(local_oVarId);
  lang_nextVarId = local_oVarId;
  return ret;
}
void lang_callFunction_void(char* functionName){
  int line;
  int local_oVarId;
  if (lang_oVarId == -1)
  {
    lang_oVarId = lang_nextVarId;
  }
  local_oVarId = lang_oVarId;
  lang_oVarId = -1;
  lang_nextArg = 'A';
  if (strcmp(functionName,"puts")==0){
    printf("%s",lang_getString("argA"));
  }
  else if (strcmp(functionName,"puti")==0){
    printf("%d",lang_getInt("argA"));
  }
  else{
    line = lang_getInt(functionName);
    if (lang_error!=1){
      lang_eval_function_void(lang_filename,line);
    }
    else{
      printf("Could not find function: %s\n",functionName);
      lang_error=1;
    }
  }
  lang_freeVariables(local_oVarId);
  lang_nextVarId = local_oVarId;
}
char* lang_callFunction_string(char* functionName){
  int line;
  char* ret = NULL;
  int local_oVarId;
  if (lang_oVarId == -1)
  {
    lang_oVarId = lang_nextVarId;
  }
  local_oVarId = lang_oVarId;
  lang_oVarId = -1;
  lang_nextArg = 'A';
  if (strcmp(functionName,"puts")==0){
    printf("%s",lang_getString("argA"));
  }
  else if (strcmp(functionName,"puti")==0){
    printf("%d",lang_getInt("argA"));
  }
  else{
    line = lang_getInt(functionName);
    if (lang_error!=1){
      ret = lang_eval_function_string(lang_filename,line);
    }
    else{
      printf("Could not find function: %s\n",functionName);
      lang_error=1;
    }
  }
  lang_freeVariables(local_oVarId);
  lang_nextVarId = local_oVarId;
  return ret;
}

void lang_dump()
{
  int ii;
  for (ii=0;ii<=lang_nextVarId;++ii)
  {
    printf("-------Variable-------\n");
    printf("Name:   %s\n",lang_var_name[ii]);
    printf("Type:   %d\n",lang_var_type[ii]);
    if (lang_var_type[ii]==LANG_INT){
      printf("Int:    %d\n",lang_var_int[ii]);
    }
    else if (lang_var_type[ii]==LANG_STRING){
      printf("String: %s\n",lang_var_string[ii]);
    }
    printf("----------------------\n");
  }
}
