#include "dispatch_extensions.h"
void positive(void){int value=3;struct ds_int_receiver i={&value};struct ds_bytes_receiver b={0};struct ds_int_interface a=ds_int_interface(&i);struct ds_byte_interface c=ds_byte_interface(&b);struct ds_view views[]={DS_RECEIVER(&i),DS_RECEIVER(&b),a.view,c.view};(void)views;}
