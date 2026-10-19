#ifndef LR_GENERIC_REFERENCE_H
#define LR_GENERIC_REFERENCE_H
static inline int lg_int(int x){return x+1;}
static inline double lg_double(double x){return x+1.0;}
static inline long lg_long(long x){return x+1L;}
/* Unary plus gives an explicit arithmetic conversion policy;arrays are rejected. */
#define LG_VALUE(x) _Generic(+(x),int:lg_int,double:lg_double)((x))
#define LG_EXTENDED(x) _Generic(+(x),int:lg_int,double:lg_double,long:lg_long)((x))
static inline int lg_object(int *p){return *p;}
static inline int lg_const_object(const int *p){return *p;}
static inline int lg_array(int (*p)[3]){return (*p)[0]+(*p)[2];}
static inline int lg_const_array(const int (*p)[3]){return (*p)[0]+(*p)[2];}
#define LG_OBJECT(x) _Generic(&(x),int*:lg_object,const int*:lg_const_object,int(*)[3]:lg_array,const int(*)[3]:lg_const_array)(&(x))
#endif
