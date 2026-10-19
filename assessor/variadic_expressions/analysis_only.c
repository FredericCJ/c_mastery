/* TRANSLATE ONLY. No main; never linked into any executable. */
#include <stdarg.h>
static int pair(int a,int b) { return a+b; }
int conflicting_sum(int i) { return i++ + i++; }
int conflicting_arguments(int i) { return pair(i++,i++); }
float wrong_request(int count,...) {
 va_list list;va_start(list,count);
 float x=va_arg(list,float); /* Actual float is promoted to double. */
 va_end(list);return x;
}
int too_late(int a,int b) { return (int)(a+b); } /* Signed overflow can precede cast. */
