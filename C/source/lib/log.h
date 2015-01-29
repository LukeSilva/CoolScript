extern int DEBUG;


#define debug_puts(x) if (DEBUG) puts(x)
#define debug_var(x,y) if (DEBUG) printf("%s: %d\n",x,y)
#define debug_str(x,y) if (DEBUG) printf("%s: '%s'\n",x,y)

