#ifndef CHECKPOINT_TRANSFER_AUTH_H
#define CHECKPOINT_TRANSFER_AUTH_H
#include <stddef.h>
#include <stdint.h>
enum { CPX_AUTH_PAYLOAD=64,CPX_AUTH_WIRE=122 };
enum cpx_auth_status { CPX_AUTH_OK,CPX_AUTH_INVALID,CPX_AUTH_FULL,CPX_AUTH_FAILED,CPX_AUTH_REPLAY,CPX_AUTH_NOT_IMPLEMENTED };
typedef int (*cpx_mac_fn)(const unsigned char *,size_t,const unsigned char *,size_t,unsigned char[32]);
struct cpx_auth_state { unsigned char key[32];uint32_t key_id;uint64_t epoch,last_sequence;unsigned seen; };
int cpx_openssl_mac(const unsigned char *,size_t,const unsigned char *,size_t,unsigned char[32]);
int cpx_tag_equal(const unsigned char[32],const unsigned char[32]);
enum cpx_auth_status cpx_auth_sign(const struct cpx_auth_state *,uint64_t,const unsigned char *,size_t,unsigned char *,size_t,size_t *,cpx_mac_fn);
enum cpx_auth_status cpx_auth_open(struct cpx_auth_state *,const unsigned char *,size_t,unsigned char *,size_t,size_t *,cpx_mac_fn);
#endif
