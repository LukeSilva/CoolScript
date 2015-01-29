#include <stdio.h>

#define LANG_NUM_VARS 400
#define LANG_DEFAULT_STRLEN 80

#define LANG_INT 0
#define LANG_STRING 1
#define LANG_FUNC 2
#define LANG_COMMENT 3
#define LANG_NOTHING 4
#define LANG_VAR 5
#define LANG_RETURN 6
#define LANG_CALL 7
#define LANG_END_FUNC 8
#define LANG_END 8
#define LANG_NUMBER 9
#define LANG_PLUS 10
#define LANG_MINUS 11
#define LANG_TIMES 12
#define LANG_DIVIDE 13
#define LANG_COMMA 14
#define LANG_ASSIGN 15
#define LANG_OBRACKET 16
#define LANG_CBRACKET 17
#define LANG_QUOTE 18
#define LANG_SEMICOLN 19
#define LANG_IF  20
#define LANG_AND 21
#define LANG_OR  22
#define LANG_GT  23
#define LANG_LT  24
#define LANG_GTE 25
#define LANG_LTE 26
#define LANG_EQ  27
#define LANG_NEQ 28 
#define LANG_NOT 29
#define LANG_WHILE 30


extern int lang_error;
void lang_dump();

int lang_findToken(FILE* lang_in);
void lang_skipWhitespace(FILE* lang_in);
char* lang_getVarName(FILE* lang_in);
char lang_parseInt(FILE* lang_in);
char lang_parseString(FILE* lang_in);
int lang_getVarId(char* name);
int lang_getVarType(int id);
char lang_setVar_int(int id, int val);
char lang_setVar_string(int id, char* val);


char lang_loadFile(char* filename);
int lang_callFunction_int(char* functionName);
char* lang_callFunction_string(char* functionName);
void lang_callFunction_void(char* functionName);
int lang_getInt(char* variableName);
char* lang_getString(char* variableName);
char lang_addArgument_int(int arg);
char lang_addArgument_string(char* arg);
