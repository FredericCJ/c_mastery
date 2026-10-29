#include "bits.h"
#include <stdio.h>
#define T(x) do{if(!(x)){fprintf(stderr,"FB_FAIL %d %s\n",__LINE__,#x);return 1;}}while(0)
static int bitops(void){
 fb_word out=917;struct fb_stats stats={91,92,93};
 T(fb_shift(16,0x1234,-1,FB_LEFT,&out)==FB_RANGE&&out==917);
 T(fb_shift(16,0x1234,16,FB_LEFT,&out)==FB_RANGE&&out==917);
 T(fb_shift(16,0x1234,0,FB_ROTATE_LEFT,&out)==FB_OK&&out==0x1234);
 T(fb_shift(16,1,15,FB_LEFT,&out)==FB_OK&&out==0x8000);
 T(fb_shift(16,0x1234,4,FB_ROTATE_LEFT,&out)==FB_OK&&out==0x2341);
 T(fb_shift(8,0x1234,4,FB_RIGHT,&out)==FB_OK&&out==3);
 T(fb_shift(8,0x1234,4,FB_ROTATE_LEFT,&out)==FB_OK&&out==0x43);
 T(fb_extract(16,0x1234,0,0,&out)==FB_OK&&out==0);
 T(fb_extract(16,0x1234,0,16,&out)==FB_OK&&out==0x1234);
 out=917;T(fb_extract(16,1,7,6,&out)==FB_RANGE&&out==917);
 T(fb_insert(16,0x1234,(struct fb_edit){4,8,10},&out)==FB_OK&&out==0x12a4);
 T(fb_insert(16,0x1234,(struct fb_edit){16,16,0},&out)==FB_OK&&out==0x1234);
 out=917;T(fb_insert(16,0x1234,(struct fb_edit){16,16,1},&out)==FB_RANGE&&out==917);
 T(fb_insert(16,0x1234,(struct fb_edit){0,4,16},&out)==FB_RANGE&&out==917);
 T(fb_patch(8,0xa0,(struct fb_edit){0,4,15},(struct fb_edit){2,6,0},FB_SEQUENTIAL,&out)==FB_OK&&out==0x83);
 out=917;T(fb_patch(8,0xa0,(struct fb_edit){0,4,15},(struct fb_edit){2,6,0},FB_SIMULTANEOUS,&out)==FB_CONFLICT&&out==917);
 T(fb_patch(8,0xa0,(struct fb_edit){0,4,15},(struct fb_edit){2,6,3},FB_SIMULTANEOUS,&out)==FB_OK&&out==0x8f);
 T(fb_stats(16,0x1234,&stats)==FB_OK&&stats.population==5&&stats.width==13&&stats.leading==3);
 T(fb_stats(8,0x100,&stats)==FB_OK&&stats.population==0&&stats.width==0&&stats.leading==8);
 T(fb_shift(0,1,0,FB_LEFT,&out)==FB_INVALID);
 puts("FB_BITS_PASS logical_mask counts half_open overlap zero_stats");return 0;
}
static int codecs(void){
 unsigned char wire[12]={0};fb_word out=917;
 T(fb_encode(UINT32_C(0x12345678),FB_BIG,wire,12)==FB_OK&&wire[0]==18&&wire[1]==52&&wire[2]==86&&wire[3]==120);
 T(fb_decode((struct fb_span){wire,4},FB_BIG,&out)==FB_OK&&out==UINT32_C(0x12345678));
 T(fb_encode(UINT32_C(0x12345678),FB_LITTLE,wire,12)==FB_OK&&wire[0]==120&&wire[3]==18);
 T(fb_decode((struct fb_span){wire,4},FB_LITTLE,&out)==FB_OK&&out==UINT32_C(0x12345678));
 unsigned offsets[]={0,1,3};
 for(unsigned k=0;k<3;++k){unsigned off=offsets[k];T(fb_encode(UINT32_C(0x12345678),FB_BIG,wire+off,12-off)==FB_OK);
  for(unsigned n=0;n<4;++n){out=917;T(fb_decode((struct fb_span){wire+off,n},FB_BIG,&out)==FB_SHORT&&out==917);}
  T(fb_decode((struct fb_span){wire+off,4},FB_BIG,&out)==FB_OK&&out==UINT32_C(0x12345678));}
 unsigned char a[]={18,52},b[]={86,120};
 T(fb_decode_two((struct fb_span){a,2},(struct fb_span){b,2},FB_BIG,&out)==FB_OK&&out==UINT32_C(0x12345678));
 out=917;T(fb_decode_two((struct fb_span){a,2},(struct fb_span){b,1},FB_BIG,&out)==FB_SHORT&&out==917);
 T(fb_decode_two((struct fb_span){NULL,0},(struct fb_span){wire+3,4},FB_BIG,&out)==FB_OK&&out==UINT32_C(0x12345678));
 out=917;T(fb_decode((struct fb_span){NULL,1},FB_BIG,&out)==FB_INVALID&&out==917);
 unsigned char known[]={1,1,0,8,18,52,86,120};struct fb_record record={917,93,94};
 T(fb_record((struct fb_span){known,8},FB_REJECT_UNKNOWN,&record)==FB_OK&&record.value==UINT32_C(0x12345678)&&record.flags==1&&record.consumed==8);
 unsigned char unknown[]={2,255,0,4};record=(struct fb_record){917,93,94};
 T(fb_record((struct fb_span){unknown,4},FB_REJECT_UNKNOWN,&record)==FB_UNSUPPORTED&&record.value==917&&record.consumed==94);
 T(fb_record((struct fb_span){unknown,4},FB_SKIP_UNKNOWN,&record)==FB_SKIPPED&&record.value==0&&record.consumed==4);
 unknown[0]=1;T(fb_record((struct fb_span){unknown,4},FB_SKIP_UNKNOWN,&record)==FB_SKIPPED&&record.consumed==4);
 unknown[3]=8;record.value=917;T(fb_record((struct fb_span){unknown,4},FB_SKIP_UNKNOWN,&record)==FB_SHORT&&record.value==917);
 puts("FB_CODEC_PASS endian offsets split_spans reject_skip unknown_payload_unread");return 0;
}
static int protocols(void){
 struct fb_register r;T(fb_register_init(&r,0xa5,0)==FB_OK);
 unsigned old=fb_register_read(&r);fb_register_write(&r,(old&~15u)|3u);T(r.state==3&&r.reads==1&&r.writes==1);
 T(fb_register_init(&r,0xa5,0)==FB_OK);T(fb_register_update(&r,3,0)==FB_OK&&r.state==0xa3&&r.reads==0);
 T(fb_register_update(&r,4,0x20)==FB_OK&&r.state==0x84);
 T(fb_register_init(&r,0xa5,1)==FB_OK);T(fb_register_read(&r)==0xa5&&r.state==5);T(fb_register_read(&r)==5);
 T(fb_register_init(&r,0xa5,1)==FB_OK);T(fb_register_update(&r,3,0)==FB_OK&&r.state==0xa3&&r.reads==0);
 unsigned char v1[]={0,1,0,8,0,0,0,1};unsigned char v2[]={0,2,0,12,0,0,0,1,0,0,4,0};struct fb_plugin p={91,92,93,94};
 T(fb_negotiate((struct fb_span){v1,8},&p)==FB_OK&&p.version==1&&p.size==8&&p.features==1&&p.max_items==0);
 T(fb_negotiate((struct fb_span){v2,12},&p)==FB_OK&&p.version==2&&p.size==12&&p.max_items==1024);
 for(unsigned n=0;n<12;++n){p=(struct fb_plugin){91,92,93,94};T(fb_negotiate((struct fb_span){v2,n},&p)==FB_SHORT&&p.version==91&&p.max_items==94);}
 v2[1]=3;T(fb_negotiate((struct fb_span){v2,12},&p)==FB_UNSUPPORTED&&p.version==91);
 v2[1]=2;v2[7]=2;T(fb_negotiate((struct fb_span){v2,12},&p)==FB_UNSUPPORTED&&p.version==91);
 puts("FB_PROTOCOL_PASS W1C RTC no_unrequested_read bounded_version_size");return 0;
}
int main(void){if(bitops()||codecs()||protocols())return 1;puts("FB_PASS bit_contract");return 0;}
