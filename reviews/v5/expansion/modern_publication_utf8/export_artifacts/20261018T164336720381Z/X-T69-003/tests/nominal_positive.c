#include "dispatch_extensions.h"
void positive(void){int value=3;DSIntReceiver i={&value};DSBytesReceiver b={0};DSIntInterface a=ds_int_interface(&i);DSByteInterface c=ds_byte_interface(&b);DSView views[]={DS_RECEIVER(&i),DS_RECEIVER(&b),a.view,c.view};(void)views;}
