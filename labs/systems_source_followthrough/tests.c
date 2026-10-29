#include "hex.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"SOURCE_HEX_ASSERT:%d:%s\n",__LINE__,#x);return 1;} }while(0)
static const char end[] = ":00000001FF\n";
static int reject(const char *s,enum sf_mode mode,enum sf_status status)
{
 struct sf_image x,before;memset(&x,0xA5,sizeof x);memcpy(&before,&x,sizeof x);
 return sf_hex(s,strlen(s),mode,&x)==status&&!memcmp(&x,&before,sizeof x);
}
int main(void)
{
 struct sf_image x;
 CHECK(sf_hex(end,strlen(end),SF_PLAIN,&x)==SF_OK&&x.count==0&&x.entry_kind==0);
 CHECK(sf_hex(":021000000102EB\n:02101000FEFFE1\n:00000001FF\n",44,SF_PLAIN,&x)==SF_OK);
 CHECK(x.count==4&&x.bytes[0].address==0x1000&&x.bytes[3].address==0x1011&&x.bytes[3].value==255);
 const char *seg=":020000021234B6\n:02FFFF00AABB9B\n:0400000312345678E5\n:00000001FF\n";
 CHECK(sf_hex(seg,strlen(seg),SF_SEGMENT,&x)==SF_OK&&x.count==2);
 CHECK(x.bytes[0].address==0x2233F&&x.bytes[1].address==0x12340&&x.entry_kind==3&&x.entry==0x12345678);
 const char *lin=":02000004FFFFFC\n:02FFFF00AABB9B\n:0400000512345678E3\n:00000001FF\n";
 CHECK(sf_hex(lin,strlen(lin),SF_LINEAR,&x)==SF_OK&&x.bytes[0].address==UINT32_MAX&&x.bytes[1].address==0&&x.entry_kind==5&&x.entry==0x12345678);
 CHECK(reject(":0000000100\n",SF_PLAIN,SF_CHECKSUM));
 CHECK(reject(":0100000001FE\n:0100000001FE\n:00000001FF\n",SF_PLAIN,SF_DUPLICATE));
 CHECK(reject(":020000021234B6\n:00000001FF\n",SF_LINEAR,SF_UNSUPPORTED));
 CHECK(reject(":02FFFF00AABB9B\n:00000001FF\n",SF_PLAIN,SF_UNSUPPORTED));
 CHECK(reject(":00000001FF\n:00000001FF\n",SF_PLAIN,SF_FORMAT));
 CHECK(reject(":00000001FF\r",SF_PLAIN,SF_FORMAT));
 CHECK(reject(":00000001F\n",SF_PLAIN,SF_FORMAT));
 CHECK(reject(":00000006FA\n:00000001FF\n",SF_PLAIN,SF_UNSUPPORTED));
 CHECK(sf_hex(":00000001ff\r\n",13,SF_PLAIN,&x)==SF_OK);
 CHECK(sf_hex(NULL,0,SF_PLAIN,&x)==SF_INVALID);
 const char *changed=":020000040001F9\n:02FFFF00AABB9B\n:00000001FF\n";
 CHECK(sf_hex(changed,strlen(changed),SF_LINEAR,&x)==SF_OK);
 CHECK(sf_window(&x,0x12340,0x20000)==SF_RANGE);
 CHECK(sf_window(&x,0x1FFFF,0x20001)==SF_OK);
 CHECK(sf_window(&x,0x20000,0x20000)==SF_INVALID);
 CHECK(sf_window(&x,0,UINT64_C(4294967297))==SF_INVALID);
 CHECK(sf_hex(seg,strlen(seg),SF_SEGMENT,&x)==SF_OK&&sf_window(&x,0x12340,0x20000)==SF_RANGE);
 CHECK(sf_hex(lin,strlen(lin),SF_LINEAR,&x)==SF_OK&&sf_window(&x,0,UINT64_C(4294967296))==SF_OK);
 puts("SOURCE_HEX_PASS:all six record mechanisms, wrap distinctions, atomic rejection");return 0;
}
