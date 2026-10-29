#include "target.h"
int ade_vulnerable(const struct ade_record *record,const unsigned char key[32],const unsigned char tag[32],struct ade_service *service){
    struct ade_record recognized=*record;recognized.count=0;
    for(unsigned i=0;i<record->count;i++)if(record->fields[i].tag<=2)recognized.fields[recognized.count++]=record->fields[i];
    int result=ade_check(&recognized,key,tag);if(result!=ADE_OK)return result;
    /* A newer endpoint interprets a field the older authenticator discarded. */
    unsigned admin=0;uint32_t amount=0;
    for(unsigned i=0;i<record->count;i++){
        const struct ade_field *field=&record->fields[i];
        if(field->tag==1)amount=(uint32_t)field->bytes[0]*256+field->bytes[1];
        if(field->tag==9&&field->length==1)admin=field->bytes[0];
    }
    if(service->total>UINT32_MAX-amount||(admin&&service->privileged==UINT32_MAX))return ADE_RANGE;
    service->total+=amount;if(admin)service->privileged++;return ADE_OK;
}
