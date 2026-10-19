#include "checks.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TF_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 static const unsigned char seeds[][12]={{1,2,0,1,0,2},{1,2,0,1},{1,0},{1,5},{0,0},{1,0,9},{1,4,255,255,255,255,255,255,255,255},{2,1,1,1,2,77}};
 const size_t lengths[]={6,4,2,2,2,3,10,6};
 for(unsigned i=0;i<8;i++)CHECK(one_case(seeds[i],lengths[i]));
 CHECK(one_case(NULL,0));TfPacket out={0};CHECK(tf_parse(0,NULL,SIZE_MAX,&out)==TF_LENGTH);
 /* Exercise original framing next to changed-session state, never reset per message. */
 TfSession s;tf_session_init(&s);CHECK(tf_session_begin(&s,1)==TF_OK);
 CHECK(tf_session_apply(&s,1,0,seeds[0],6,&out)==TF_OK&&s.total==3&&s.next==1);
 TfSession old=s;TfPacket old_out=out;
 CHECK(tf_session_apply(&s,1,0,seeds[0],6,&out)==TF_SEQUENCE&&memcmp(&s,&old,sizeof s)==0&&memcmp(&out,&old_out,sizeof out)==0);
 CHECK(tf_session_apply(&s,1,1,seeds[1],4,&out)==TF_LENGTH&&memcmp(&s,&old,sizeof s)==0);
 CHECK(tf_session_apply(&s,1,1,seeds[0],6,&out)==TF_OK&&s.total==6);
 CHECK(tf_session_begin(&s,2)==TF_OK&&s.total==0&&s.next==0);
 CHECK(tf_session_apply(&s,1,0,seeds[0],6,&out)==TF_SESSION);
 CHECK(tf_session_apply(&s,2,0,seeds[0],6,&out)==TF_OK&&s.total==3);
 s.next=UINT16_MAX;CHECK(tf_session_apply(&s,2,UINT16_MAX,seeds[2],2,&out)==TF_OK);old=s;
 CHECK(tf_session_apply(&s,2,UINT16_MAX,seeds[2],2,&out)==TF_SEQUENCE&&memcmp(&s,&old,sizeof s)==0);
 CHECK(tf_session_begin(&s,3)==TF_OK);s.total=UINT32_MAX;old=s;
 CHECK(tf_session_apply(&s,3,0,seeds[0],6,&out)==TF_RANGE&&memcmp(&s,&old,sizeof s)==0);
 puts("TF_PASS:grammar,reset,optional,session,sequence,bounds,publication");return 0;
}
