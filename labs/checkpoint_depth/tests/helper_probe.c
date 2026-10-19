/* Original inspection specimen: a valid large aggregate copy can emit a helper. */
typedef struct { unsigned char bytes[65536]; } CPWLarge;
void cpw_copy(CPWLarge *destination,const CPWLarge *source){*destination=*source;}
