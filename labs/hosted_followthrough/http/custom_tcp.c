#define main retained_network_test_main
#include "../../communications/tests/net.c"
#undef main
int main(void) {
  CHECK(!http());
  struct net_stack s;struct net_packet p,o;CHECK(!prepare(&s,100));
  const char request[]="GET /health HTTP/1.1\r\nHost: 192.0.2.2:8080\r\n\r\n";
  tcp(&p,101,s.snd_nxt,24,512,request,sizeof request-1);CHECK(net_receive(&s,p.bytes,p.length,30,&o)==1);
  CHECK(s.response_length && s.flight && s.response_acked==0);unsigned char stable[1600];memcpy(stable,s.response,s.response_length);
  /* Original copied packet/input storage may now be reused. Response and flight
     are owned by the stack until ACK/abort, not by the borrowed input packet. */
  memset(p.bytes,0xa5,sizeof p.bytes);CHECK(!memcmp(stable,s.response,s.response_length));
  struct net_packet flight=s.retained;uint32_t sent=s.snd_nxt;CHECK(net_tick(&s,s.due,&o)==1);
  CHECK(s.snd_nxt==sent && s.response_acked==0 && o.length==flight.length && !memcmp(o.bytes,flight.bytes,o.length));
  CHECK(!memcmp(stable,s.response,s.response_length));
  CHECK(!memcmp(stable,"HTTP/1.1 200 OK\r\n",17));
  char text[1601];memcpy(text,stable,s.response_length);text[s.response_length]=0;CHECK(!strstr(text,"Date:"));
  printf("CUSTOM_HTTP_PASS:borrow-call-only,response-retained-until-ACK,retransmit-identical,no-Date,existing-com_net;response=%zu;sent=%zu;acked=%zu\n",s.response_length,s.response_sent,s.response_acked);return 0;
}
