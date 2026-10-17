#include "errors.h"
/* Compiling learner scaffold: implement each contract before its semantic gate can pass. */
er_result er_ok(uint32_t value) {
    (void)value;
    er_result r = { ER_NOT_IMPLEMENTED, { .value = 0 } }; return r;
}
er_result er_error(er_code code, size_t offset) {
    (void)code; (void)offset;
    er_result r = { ER_NOT_IMPLEMENTED, { .value = 0 } }; return r;
}
er_code er_optional_pack(uint32_t value, int present, uint32_t *encoded) {
    (void)value; (void)present; (void)encoded;
    return ER_NOT_IMPLEMENTED;
}
er_result er_optional_unpack(uint32_t encoded) {
    (void)encoded;
    er_result r = { ER_NOT_IMPLEMENTED, { .value = 0 } }; return r;
}
er_result er_parse_pair(const unsigned char *bytes, size_t length, uint32_t *sum) {
    (void)bytes; (void)length; (void)sum;
    er_result r = { ER_NOT_IMPLEMENTED, { .value = 0 } }; return r;
}
er_progress er_transfer(const unsigned char *bytes, size_t length, size_t next,                         er_sink sink, void *context) {
    (void)bytes; (void)length; (void)next; (void)sink; (void)context;
    er_progress p = {0, ER_NOT_IMPLEMENTED}; return p;
}
er_result er_map(er_result input, uint32_t (*fn)(uint32_t, void *), void *context) {
    (void)input; (void)fn; (void)context;
    er_result r = { ER_NOT_IMPLEMENTED, { .value = 0 } }; return r;
}
er_result er_then(er_result input, er_result (*fn)(uint32_t, void *), void *context) {
    (void)input; (void)fn; (void)context;
    er_result r = { ER_NOT_IMPLEMENTED, { .value = 0 } }; return r;
}
er_validation er_validate(er_config input, er_config *output, unsigned *errors, size_t capacity) {
    (void)input; (void)output; (void)errors; (void)capacity;
    er_validation v = {SIZE_MAX,0,0}; return v;
}
er_code er_defer(er_frame *frame, er_cleanup fn, void *context) {
    (void)frame; (void)fn; (void)context;
    return ER_NOT_IMPLEMENTED;
}
er_code er_raise(er_frame *frame, er_code code) {
    (void)frame; (void)code;
    return ER_NOT_IMPLEMENTED;
}
er_result er_run(er_frame *frame, er_result (*body)(er_frame *, void *), void *context) {
    (void)frame; (void)body; (void)context;
    er_result r = { ER_NOT_IMPLEMENTED, { .value = 0 } }; return r;
}
er_code er_add16(uint32_t a, uint32_t b, er_policy policy, uint32_t *output) {
    (void)a; (void)b; (void)policy; (void)output;
    return ER_NOT_IMPLEMENTED;
}
er_code er_serial16(uint32_t a, uint32_t b, int *order) {
    (void)a; (void)b; (void)order;
    return ER_NOT_IMPLEMENTED;
}
er_code er_contract_counter(er_mode mode, int amount, int *counter, size_t *reports) {
    (void)mode; (void)amount; (void)counter; (void)reports;
    return ER_NOT_IMPLEMENTED;
}
er_code er_refine(uint32_t numerator, uint32_t denominator, er_fraction *output) {
    (void)numerator; (void)denominator; (void)output;
    return ER_NOT_IMPLEMENTED;
}
