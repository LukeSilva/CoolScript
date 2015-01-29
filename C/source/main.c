#include <stdio.h>
#include "lib/language.h"
int main(){
  lang_loadFile("script");
  lang_addArgument_int(5);
  lang_addArgument_int(2);
  lang_addArgument_string("Calling myFunction from main\n");
  lang_dump();
  int ret;
  ret = lang_callFunction_int("myFunction");
  printf("Ret: %d\n",ret);
  return 0;
}
