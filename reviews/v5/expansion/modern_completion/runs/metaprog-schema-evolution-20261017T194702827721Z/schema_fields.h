#ifndef CMASTERY_MPS_FIELDS_H
#define CMASTERY_MPS_FIELDS_H
/* The declared schema is supplied; derive every operation from this list.
 * index,name,C type,tag,wire bytes,required,kind,min,max,first version */
#define MPS_VERSION 3
#define MPS_FIELDS(X) \
 X(0,code,uint16_t,1,2,1,U16,0,65535,1) \
 X(1,value,uint32_t,2,4,1,U32,0,UINT32_MAX,1) \
 X(2,note,int32_t,3,5,0,S32,INT32_MIN,INT32_MAX,2) \
 X(3,quality,uint16_t,4,2,0,U16,0,15,3)
#endif
