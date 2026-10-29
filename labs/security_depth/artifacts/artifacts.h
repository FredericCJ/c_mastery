#ifndef SA_ARTIFACTS_H
#define SA_ARTIFACTS_H
#include <stddef.h>
#include <stdint.h>
#include "command.h"
struct sa_trust { unsigned char public_key[32]; uint32_t epoch,minimum; };
struct sa_artifact { unsigned char digest[32],public_key[32],signature[64]; uint32_t version,epoch; unsigned committed; };
struct sa_report { unsigned digest_matches,signature_valid,trusted_signer,compatible,generation_current,committed,ready; };
int sa_digest(const unsigned char *,size_t,unsigned char[32]);
void sa_manifest(const struct sa_artifact *,unsigned char[44]);
void sa_transition(const struct sa_trust *,const unsigned char[32],uint32_t,uint32_t,unsigned char[49]);
int sa_verify(const struct sa_trust *,const struct sa_artifact *,const unsigned char *,size_t,struct sa_report *);
int sa_rotate(struct sa_trust *,const unsigned char[32],uint32_t,uint32_t,const unsigned char[64]);
struct sa_service { struct sc_service queue; unsigned used_bytes,charges[3],resource[3]; unsigned char staging[3][4]; };
void sa_service_init(struct sa_service *);
enum sc_result sa_submit(struct sa_service *,const struct sa_report *,const struct sc_authority *,const unsigned char *,size_t,struct sc_ticket *);
enum sc_result sa_verified_submit(struct sa_service *,const struct sa_trust *,const struct sa_artifact *,const unsigned char *,size_t,const struct sc_authority *,const unsigned char *,size_t,struct sc_ticket *);
enum sc_result sa_finish(struct sa_service *,struct sc_ticket,int,const struct sc_authority *,int,uint32_t *,uint32_t *,char[4]);
enum sc_result sa_shutdown(struct sa_service *);
#endif
