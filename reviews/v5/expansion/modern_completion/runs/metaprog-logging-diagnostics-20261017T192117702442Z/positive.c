#include "logging_macros.h"
#include <stdio.h>
int probe1(MplSink *s){return MPL_LOG(s,mpl_number(1));}
int probe2(MplSink *s){return MPL_LOG(s,mpl_number(1),mpl_number(2));}
int probe3(MplSink *s){return MPL_LOG(s,mpl_number(1),mpl_number(2),mpl_number(3));}
int probe4(MplSink *s){return MPL_LOG(s,mpl_number(1),mpl_number(2),mpl_number(3),mpl_number(4));}
int main(void){MplSink s={0};if(probe1(&s)||probe2(&s)||probe3(&s)||probe4(&s)||s.records[3].count!=4)return 1;s.count=0;if(MPL_LOG(&s,((MplField){MPL_NUMBER,19,NULL,0}))||MPL_LOG0(&s)||MPL_LOG_AT0(&s)||MPL_LOG(&s,mpl_number(3.5)))return 2;if(s.records[0].fields[0].number!=19||s.records[1].count||!s.records[2].has_site||s.records[3].fields[0].number!=3)return 3;puts("MPL_DIAGNOSTICS positive typed-arities zero protected-comma implicit-conversion");return 0;}
