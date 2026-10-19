/* Diagnostic experiment: default does not list the ERROR alternative. */
enum Tag { INTEGER, TEXT, ERROR };
int enum_consumer(enum Tag tag){
 switch(tag){case INTEGER:return 1;case TEXT:return 2;
#ifdef BF_COMPLETE
 case ERROR:return 3;
#endif
 default:return 0;}
}
int unsigned_consumer(unsigned tag){switch(tag){case 0:return 1;case 1:return 2;default:return 0;}}
