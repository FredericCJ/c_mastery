#define _POSIX_C_SOURCE 200809L
#include "artifacts.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x)                                   \
    do                                             \
    {                                              \
        if (!(x))                                  \
        {                                          \
            fprintf(stderr, "SA_ASSERT:%s\n", #x); \
            return 41;                             \
        }                                          \
    } while (0)
static EVP_PKEY *owned_keys[2];
static unsigned owned_count;
static FILE *owned_log;

static int keys(unsigned char seed, EVP_PKEY **key, unsigned char pub[32])
{
    unsigned char private_bytes[32];
    size_t n = 32;
    memset(private_bytes, seed, sizeof private_bytes);
    *key = EVP_PKEY_new_raw_private_key_ex(NULL, "ED25519", NULL, private_bytes,
                                           sizeof private_bytes);
    if (*key && owned_count < 2)
        owned_keys[owned_count++] = *key;
    return *key && EVP_PKEY_get_raw_public_key(*key, pub, &n) == 1 && n == 32;
}

static int sign(EVP_PKEY *key, const unsigned char *message, size_t n,
                unsigned char signature[64])
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    size_t count = 64;
    int ok = ctx && EVP_DigestSignInit(ctx, NULL, NULL, NULL, key) == 1 &&
             EVP_DigestSign(ctx, signature, &count, message, n) == 1 &&
             count == 64;
    EVP_MD_CTX_free(ctx);
    return ok;
}

static int sign_artifact(EVP_PKEY *key, struct sa_artifact *a)
{
    unsigned char message[44] = {0};
    sa_manifest(a, message);
    return sign(key, message, sizeof message, a->signature);
}

static int artifact(void)
{
    static const unsigned char abc[3] = {'a', 'b', 'c'},
                               abd[3] = {'a', 'b', 'd'};
    const unsigned char expected[32] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40,
        0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17,
        0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
    struct sa_artifact a = {0};
    struct sa_trust t = {{0}, 1, 2};
    struct sa_report r;
    EVP_PKEY *key = NULL, *other = NULL;
    unsigned char attacker[32];
    CHECK(keys(1, &key, t.public_key) && keys(2, &other, attacker));
    memcpy(a.public_key, t.public_key, 32);
    a.version = 2;
    a.epoch = 1;
    a.committed = 1;
    CHECK(sa_digest(abc, 3, a.digest) == 0 && !memcmp(a.digest, expected, 32));
    CHECK(sign_artifact(key, &a));
    unsigned char manifest[44];
    const unsigned char prefix[12] = {'C', 'M', '3', '5', 0, 0,
                                      0,   2,   0,   0,   0, 1};
    sa_manifest(&a, manifest);
    CHECK(!memcmp(manifest, prefix, 12) &&
          !memcmp(manifest + 12, expected, 32));
    CHECK(sa_verify(&t, &a, abc, 3, &r) == 0 && r.ready == 1);
    CHECK(sa_verify(&t, &a, abd, 3, &r) == 0 && r.digest_matches == 0 &&
          r.ready == 0);
    CHECK(sa_digest(abd, 3, a.digest) == 0);
    CHECK(sa_verify(&t, &a, abd, 3, &r) == 0 && r.digest_matches == 1 &&
          r.signature_valid == 0 && !r.ready);
    memcpy(a.public_key, attacker, 32);
    CHECK(sign_artifact(other, &a));
    CHECK(sa_verify(&t, &a, abd, 3, &r) == 0 && r.signature_valid == 1 &&
          r.trusted_signer == 0 && r.ready == 0);
    memcpy(a.public_key, t.public_key, 32);
    CHECK(sa_digest(abc, 3, a.digest) == 0);
    for (uint32_t v = 1; v <= 3; ++v)
    {
        a.version = v;
        CHECK(sign_artifact(key, &a));
        CHECK(sa_verify(&t, &a, abc, 3, &r) == 0 && r.compatible == (v >= 2) &&
              r.ready == (v >= 2));
    }
    a.committed = 0;
    CHECK(sa_verify(&t, &a, abc, 3, &r) == 0 && r.signature_valid &&
          r.compatible && !r.committed && !r.ready);
    a.committed = 1;
    a.epoch = 2;
    CHECK(sign_artifact(key, &a));
    CHECK(sa_verify(&t, &a, abc, 3, &r) == 0 && r.signature_valid &&
          !r.generation_current && !r.ready);
    puts(
        "OBS artifact digest-replacement untrusted-key old-version uncommitted distinct");
    return 0;
}

static int rotation(void)
{
    struct sa_trust t = {{0}, 1, 2}, old;
    EVP_PKEY *a = NULL, *b = NULL;
    unsigned char pub[32], message[49] = {0}, signature[64];
    CHECK(keys(1, &a, t.public_key) && keys(2, &b, pub));
    old = t;
    sa_transition(&t, pub, 2, 3, message);
    CHECK(sign(a, message, 49, signature));
    const unsigned char prefix[17] = {'R', 'O', 'T', '3', '5', 0, 0, 0, 1,
                                      0,   0,   0,   2,   0,   0, 0, 3};
    CHECK(!memcmp(message, prefix, 17) && !memcmp(message + 17, pub, 32));
    signature[0] ^= 1;
    CHECK(sa_rotate(&t, pub, 2, 3, signature) == -1 && t.epoch == old.epoch &&
          !memcmp(t.public_key, old.public_key, 32));
    signature[0] ^= 1;
    CHECK(sa_rotate(&t, pub, 2, 3, signature) == 0 && t.epoch == 2 &&
          t.minimum == 3 && !memcmp(t.public_key, pub, 32));
    CHECK(sa_rotate(&t, old.public_key, 1, 2, signature) == -1);
    struct sa_artifact item = {0};
    struct sa_report r;
    const unsigned char bytes[3] = {'a', 'b', 'c'};
    item.version = 3;
    item.epoch = 2;
    item.committed = 1;
    memcpy(item.public_key, old.public_key, 32);
    CHECK(sa_digest(bytes, 3, item.digest) == 0 && sign_artifact(a, &item));
    CHECK(sa_verify(&t, &item, bytes, 3, &r) == 0 && r.signature_valid &&
          !r.trusted_signer && !r.ready);
    memcpy(item.public_key, pub, 32);
    CHECK(sign_artifact(b, &item));
    CHECK(sa_verify(&t, &item, bytes, 3, &r) == 0 && r.ready);
    t.epoch = UINT32_MAX;
    CHECK(sa_rotate(&t, old.public_key, 0, 3, signature) == -1);
    puts("OBS rotation old-key-revoked minimum-monotonic epoch-nonwrapping");
    return 0;
}

static int service(void)
{
    struct sa_service s = {0};
    struct sa_report r = {1, 1, 1, 1, 1, 1, 1};
    struct sc_authority a = {1, 1};
    struct sc_ticket ticket, second;
    uint32_t ordinary = 0, privileged = 0;
    char out[4] = {'X', 'X', 'X', 'X'};
    const unsigned char resource[] = {6, 2, 2, 0, 1, 0, 2},
                        ordinary_bytes[] = {1, 1, 0, 7}, denied[] = {2, 0},
                        bad[] = {1, 2, 0}, wide[] = {6, 2, 3, 0, 1, 0, 2, 0, 3},
                        one[] = {6, 1, 2, 1, 2};
    FILE *log = tmpfile();
    owned_log = log;
    CHECK(log != NULL && fwrite("SAFE", 1, 4, log) == 4 && fflush(log) == 0);
    sa_service_init(&s);
    CHECK(sa_submit(&s, &r, &a, bad, sizeof bad, &ticket) == SC_LENGTH &&
          s.queue.count == 0);
    a.allowed = 0;
    CHECK(sa_submit(&s, &r, &a, denied, sizeof denied, &ticket) == SC_DENIED &&
          s.queue.count == 0);
    a.allowed = 1;
    r.ready = 0;
    CHECK(sa_submit(&s, &r, &a, ordinary_bytes, sizeof ordinary_bytes,
                    &ticket) == SC_DENIED);
    r.ready = 1;
    CHECK(sa_submit(&s, &r, &a, ordinary_bytes, sizeof ordinary_bytes,
                    &ticket) == SC_OK);
    CHECK(sa_finish(&s, ticket, 0, &a, fileno(log), &ordinary, &privileged,
                    out) == SC_OK &&
          ordinary == 7 && privileged == 0);
    CHECK(sa_submit(&s, &r, &a, wide, sizeof wide, &ticket) == SC_BUDGET &&
          s.used_bytes == 0);
    CHECK(sa_submit(&s, &r, &a, resource, sizeof resource, &ticket) == SC_OK &&
          s.used_bytes == 4);
    CHECK(sa_submit(&s, &r, &a, resource, sizeof resource, &second) ==
              SC_BUDGET &&
          s.used_bytes == 4);
    CHECK(sa_finish(&s, ticket, 0, &a, fileno(log), &ordinary, &privileged,
                    out) == SC_OK &&
          privileged == 1 && !memcmp(out, "SAFE", 4) && s.used_bytes == 0);
    for (unsigned i = 0; i < 4; ++i)
        CHECK(s.staging[ticket.index][i] == 0);
    CHECK(sa_finish(&s, ticket, 0, &a, fileno(log), &ordinary, &privileged,
                    out) == SC_STALE);
    CHECK(sa_submit(&s, &r, &a, resource, sizeof resource, &ticket) == SC_OK);
    CHECK(sc_revoke(&a) == SC_OK);
    memcpy(out, "XXXX", 4);
    CHECK(sa_finish(&s, ticket, 0, &a, -1, &ordinary, &privileged, out) ==
              SC_DENIED &&
          privileged == 1 && !memcmp(out, "XXXX", 4) && s.used_bytes == 0);
    a.allowed = 1;
    CHECK(sa_submit(&s, &r, &a, resource, sizeof resource, &ticket) == SC_OK);
    CHECK(sa_finish(&s, ticket, 0, &a, -1, &ordinary, &privileged, out) ==
              SC_INVALID &&
          s.used_bytes == 0 && privileged == 1);
    CHECK(sa_submit(&s, &r, &a, one, sizeof one, &ticket) == SC_OK &&
          s.used_bytes == 2);
    CHECK(sa_submit(&s, &r, &a, one, sizeof one, &second) == SC_OK &&
          s.used_bytes == 4);
    CHECK(sa_finish(&s, ticket, 1, &a, -1, &ordinary, &privileged, out) ==
              SC_OK &&
          s.used_bytes == 2);
    CHECK(sa_shutdown(&s) == SC_OK && s.used_bytes == 0 && s.queue.count == 0 &&
          s.queue.terminal == s.queue.released);
    unsigned terminals = s.queue.terminal;
    CHECK(sa_shutdown(&s) == SC_OK && s.queue.terminal == terminals);
    int closed = fclose(log);
    owned_log = NULL;
    CHECK(closed == 0);
    puts(
        "OBS service malformed denied quota stale descriptor cleanup success changed-width");
    return 0;
}

static void hostile_callback(struct sc_authority *authority)
{
    authority->allowed = 1;
}

static int provider_failure(void)
{
    const unsigned char payload[3] = {'a', 'b', 'c'};
    struct sa_trust trust = {{0}, 1, 2};
    struct sa_artifact artifact = {0};
    EVP_PKEY *key = NULL;
    size_t count = 0;
    struct sa_report report = {77, 78, 79, 80, 81, 82, 83};
    CHECK(keys(1, &key, trust.public_key));
    memcpy(artifact.public_key, trust.public_key, 32);
    artifact.version = 2;
    artifact.epoch = 1;
    artifact.committed = 1;
    CHECK(EVP_Q_digest(NULL, "SHA256", NULL, payload, 3, artifact.digest,
                       &count) == 1 &&
          count == 32 && sign_artifact(key, &artifact));
    CHECK(sa_verify(&trust, &artifact, payload, 3, &report) == -1);
    CHECK(report.digest_matches == 77 && report.signature_valid == 78 &&
          report.trusted_signer == 79 && report.compatible == 80 &&
          report.generation_current == 81 && report.committed == 82 &&
          report.ready == 83);
    puts("OBS provider_failure simulated-owned-boundary output-preserved");
    return 0;
}

static int integrated(void)
{
    const unsigned char payload[3] = {'a', 'b', 'c'}, command[2] = {2, 0};
    struct sa_trust trust = {{0}, 1, 2};
    struct sa_artifact artifact = {0};
    struct sa_report report;
    EVP_PKEY *key = NULL, *next_key = NULL;
    unsigned char next_public[32], transition[49], signature[64];
    struct sa_service service = {0};
    struct sc_authority authority = {1, 1};
    struct sc_ticket ticket;
    uint32_t p = 0, q = 0;
    char bytes[4] = {0};
    CHECK(keys(1, &key, trust.public_key));
    memcpy(artifact.public_key, trust.public_key, 32);
    artifact.version = 2;
    artifact.epoch = 1;
    artifact.committed = 1;
    CHECK(sa_digest(payload, 3, artifact.digest) == 0 &&
          sign_artifact(key, &artifact));
    CHECK(sa_verify(&trust, &artifact, payload, 3, &report) == 0 &&
          report.ready);
    sa_service_init(&service);
    CHECK(sa_verified_submit(&service, &trust, &artifact, payload, 3,
                             &authority, command, 2, &ticket) == SC_OK);
    CHECK(sa_finish(&service, ticket, 0, &authority, -1, &p, &q, bytes) ==
              SC_OK &&
          q == 1);
    artifact.committed = 0;
    CHECK(sa_verify(&trust, &artifact, payload, 3, &report) == 0 &&
          !report.ready);
    CHECK(sa_verified_submit(&service, &trust, &artifact, payload, 3,
                             &authority, command, 2, &ticket) == SC_DENIED &&
          q == 1);
    artifact.committed = 1;
    CHECK(sa_verify(&trust, &artifact, payload, 3, &report) == 0 &&
          report.ready);
    CHECK(keys(2, &next_key, next_public));
    sa_transition(&trust, next_public, 2, 2, transition);
    CHECK(sign(key, transition, 49, signature));
    CHECK(sa_rotate(&trust, next_public, 2, 2, signature) == 0);
    /* A cached plain report is not a revocable capability; low-level API trusts its caller. */
    CHECK(sa_submit(&service, &report, &authority, command, 2, &ticket) ==
          SC_OK);
    CHECK(sa_finish(&service, ticket, 1, &authority, -1, &p, &q, bytes) ==
              SC_OK &&
          q == 1);
    CHECK(sa_verified_submit(&service, &trust, &artifact, payload, 3,
                             &authority, command, 2, &ticket) == SC_DENIED &&
          q == 1);
    artifact.epoch = 2;
    memcpy(artifact.public_key, next_public, 32);
    CHECK(sign_artifact(next_key, &artifact));
    const unsigned char substitute[4] = {1, 1, 0, 7};
    CHECK(sa_verified_submit(&service, &trust, &artifact, payload, 3,
                             &authority, substitute, 4, &ticket) == SC_OK);
    CHECK(sa_finish(&service, ticket, 0, &authority, -1, &p, &q, bytes) ==
              SC_OK &&
          p == 7 && q == 1);
    puts(
        "OBS integrated fresh-trust-check cached-report-counterexample artifact-not-command-signature");
    return 0;
}

static int shared(void)
{
    struct sa_service s = {0};
    struct sa_report report = {1, 1, 1, 1, 1, 1, 1};
    struct sc_authority authority = {1, 0};
    struct sc_ticket t;
    const unsigned char request[2] = {2, 0};
    uint32_t p = 0, q = 0;
    char out[4] = {0};
    sa_service_init(&s);
    CHECK(sa_submit(&s, &report, &authority, request, 2, &t) == SC_DENIED);
    hostile_callback(&authority);
    CHECK(sa_submit(&s, &report, &authority, request, 2, &t) == SC_OK);
    CHECK(sa_finish(&s, t, 0, &authority, -1, &p, &q, out) == SC_OK && q == 1);
    puts(
        "OBS shared-memory-trust-boundary raw-write grants synthetic effect no-privilege-escalation");
    return 0;
}

int main(int argc, char **argv)
{
    int result = 2;
    if (argc != 2)
        return result;
    if (!strcmp(argv[1], "artifact"))
        result = artifact();
    else if (!strcmp(argv[1], "rotation"))
        result = rotation();
    else if (!strcmp(argv[1], "service"))
        result = service();
    else if (!strcmp(argv[1], "shared"))
        result = shared();
    else if (!strcmp(argv[1], "integrated"))
        result = integrated();
    else if (!strcmp(argv[1], "provider_failure"))
        result = provider_failure();
    while (owned_count)
        EVP_PKEY_free(owned_keys[--owned_count]);
    if (owned_log && fclose(owned_log))
        return 42;
    return result;
}
