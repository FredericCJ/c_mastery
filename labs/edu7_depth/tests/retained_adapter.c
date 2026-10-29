/* Host composition only: explicit retained learner/assessor codec source path.
   No library implements the new grant/send/session mechanisms. */
#include "com_edu7.h"
#include "edu7_depth.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "ADAPTER_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

static int exchange(struct e7_session *s, unsigned sender, uint16_t cp,
                    const uint8_t *bytes, size_t n, int16_t *set,
                    unsigned *effects, uint8_t *out, size_t *outlen)
{
    uint32_t token;
    uint8_t cached[80], encoded[80];
    size_t cn = 0, en = 0;
    struct edu_record request, response;
    int16_t staged = *set;
    int r = e7_session_prepare(s, sender, 1, 1, cp, bytes, n, &token, cached,
                               sizeof cached, &cn);
    if (r == E7_DUPLICATE)
    {
        memcpy(out, cached, cn);
        *outlen = cn;
        return 0;
    }
    if (r)
        return r;
    if (edu_record_decode(bytes, n, &request))
    {
        CHECK(e7_session_commit(s, token, 0, NULL, 0) == E7_OK);
        return E7_REJECTED;
    }
    if (edu_application(&request, &staged, &response) ||
        edu_record_encode(&response, encoded, &en))
    {
        CHECK(e7_session_commit(s, token, 0, NULL, 0) == E7_OK);
        return E7_REJECTED;
    }
    CHECK(en <= sizeof encoded);
    CHECK(e7_session_commit(s, token, 1, encoded, en) == E7_OK);
    if (request.verb == 2 && response.verb == 3)
        (*effects)++;
    *set = staged;
    memcpy(out, encoded, en);
    *outlen = en;
    return 0;
}

int main(void)
{
    struct e7_session s = {0};
    struct edu_record request = {2, 42, -4000, 1, 1, {0}, 0}, response;
    uint8_t wire[80], reply[80], first[80];
    size_t n, rn, fn;
    int16_t set = 2200;
    unsigned effects = 0;
    CHECK(e7_session_open(&s, 1, 1, 1, 1) == E7_OK);
    CHECK(!edu_record_encode(&request, wire, &n));
    CHECK(!exchange(&s, 17, 1, wire, n, &set, &effects, reply, &rn) &&
          set == -4000 && effects == 1);
    memcpy(first, reply, rn);
    fn = rn;
    CHECK(!exchange(&s, 17, 1, wire, n, &set, &effects, reply, &rn) &&
          effects == 1 && rn == fn && !memcmp(first, reply, rn));
    wire[0] = 2;
    CHECK(exchange(&s, 17, 2, wire, n, &set, &effects, reply, &rn) ==
              E7_REJECTED &&
          s.receipt[0].checkpoint == 1 && effects == 1);
    request.value = 12501;
    CHECK(!edu_record_encode(&request, wire, &n));
    CHECK(!exchange(&s, 17, 2, wire, n, &set, &effects, reply, &rn) &&
          set == -4000 && effects == 1);
    CHECK(!edu_record_decode(reply, rn, &response) && response.verb == 4 &&
          response.value == -4000);
    request.verb = 1;
    request.has_value = request.has_unit = 0;
    CHECK(!edu_record_encode(&request, wire, &n));
    CHECK(!exchange(&s, 17, 3, wire, n, &set, &effects, reply, &rn));
    CHECK(!edu_record_decode(reply, rn, &response) && response.value == 1800);
    puts("retained codec composition: staged invalid syntax, cached duplicate "
         "response, application rejection and GET passed");
    return 0;
}
