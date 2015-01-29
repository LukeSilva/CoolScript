#include <stdio.h>
#include "language.h"
char lang_eval_stmt_int(FILE* lang_in);
char* lang_eval_stmt_string(FILE* lang_in);
int lang_eval_function_int(char* filename,int line);
char* lang_eval_function_string(char* filename,int line);
void lang_eval_function_void(char* filename, int id);
