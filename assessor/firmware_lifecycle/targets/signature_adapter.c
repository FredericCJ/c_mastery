#include "signature_adapter.h"
#include <openssl/evp.h>
#ifndef SIGN_MUTANT
#define SIGN_MUTANT 0
#endif
struct lce_signature_result lce_ed25519(const struct lce_keyring*ring,unsigned id,const unsigned char*p,size_t n,const unsigned char*sig,size_t z){struct lce_signature_result result={0,0};if(!ring||id<1||id>2||!(ring->present&(1u<<id))||!p||!sig||z!=64)return result;EVP_PKEY*key=EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519,NULL,ring->public_keys[id-1],32);EVP_MD_CTX*ctx=EVP_MD_CTX_new();if(key&&ctx&&EVP_DigestVerifyInit(ctx,NULL,NULL,NULL,key)==1){int verdict=EVP_DigestVerify(ctx,sig,z,p,n);if(verdict==1||SIGN_MUTANT==1){result.accepted=1;result.verified_key=SIGN_MUTANT==2?3-id:id;}}EVP_MD_CTX_free(ctx);EVP_PKEY_free(key);return result;}
int lce_signature_bridge(void*ctx,unsigned id,const unsigned char*p,size_t n,const unsigned char*s,size_t z){struct lce_signature_result r=lce_ed25519(ctx,id,p,n,s,z);return r.accepted==1&&r.verified_key==id;}
