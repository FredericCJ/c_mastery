#ifndef SA_ARTIFACTS_H
#define SA_ARTIFACTS_H
#include <stddef.h>
#include <stdint.h>
#include "command.h"
typedef struct { unsigned char public_key[32]; uint32_t epoch,minimum; } SATrust;
typedef struct { unsigned char digest[32],public_key[32],signature[64]; uint32_t version,epoch; unsigned committed; } SAArtifact;
typedef struct { unsigned digest_matches,signature_valid,trusted_signer,compatible,generation_current,committed,ready; } SAReport;
int sa_digest(const unsigned char *,size_t,unsigned char[32]);
void sa_manifest(const SAArtifact *,unsigned char[44]);
void sa_transition(const SATrust *,const unsigned char[32],uint32_t,uint32_t,unsigned char[49]);
int sa_verify(const SATrust *,const SAArtifact *,const unsigned char *,size_t,SAReport *);
int sa_rotate(SATrust *,const unsigned char[32],uint32_t,uint32_t,const unsigned char[64]);
typedef struct { SCService queue; unsigned used_bytes,charges[3],resource[3]; unsigned char staging[3][4]; } SAService;
void sa_service_init(SAService *);
enum sc_result sa_submit(SAService *,const SAReport *,const SCAuthority *,const unsigned char *,size_t,SCTicket *);
enum sc_result sa_verified_submit(SAService *,const SATrust *,const SAArtifact *,const unsigned char *,size_t,const SCAuthority *,const unsigned char *,size_t,SCTicket *);
enum sc_result sa_finish(SAService *,SCTicket,int,const SCAuthority *,int,uint32_t *,uint32_t *,char[4]);
enum sc_result sa_shutdown(SAService *);
#endif
