#include "replacement.h"
void le_drop(LeOwner *owner){
 if(!owner || !owner->data)return;
 LeOwner old=*owner;*owner=(LeOwner){0};old.ops.release(old.ops.context,old.id,old.data,old.size);
}
LeStatus le_run(LeOps ops,unsigned transfer,LeOwner *out){
 if(!out || out->data || !ops.acquire || !ops.release || transfer>2)return LE_INVALID;
 [[gnu::cleanup(le_drop)]] LeOwner a={0};
 [[gnu::cleanup(le_drop)]] LeOwner b={0};
 a=(LeOwner){.data=ops.acquire(ops.context,1,8),.size=8,.id=1,.ops=ops};if(!a.data)return LE_NOMEM;
 b=(LeOwner){.data=ops.acquire(ops.context,2,16),.size=16,.id=2,.ops=ops};if(!b.data)return LE_NOMEM;
 for(size_t i=0;i<a.size;i++)a.data[i]=(unsigned char)(i+1);
 for(size_t i=0;i<b.size;i++)b.data[i]=(unsigned char)(i+17);
 if(transfer){LeOwner *selected=transfer==1?&a:&b;*out=*selected;*selected=(LeOwner){0};}
 return LE_OK;
}
