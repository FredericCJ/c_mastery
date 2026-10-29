#ifndef CMASTERY_FORMAT_STREAM_H
#define CMASTERY_FORMAT_STREAM_H
#include "metaprog.h"
enum { MFP_OK, MFP_INVALID, MFP_FORMAT, MFP_TYPE, MFP_FULL, MFP_MORE,
       MFP_AGAIN, MFP_IO, MFP_PROTOCOL, MFP_CANCELLED, MFP_BUSY };
enum { MFP_NUMBER=1, MFP_BYTES=2, MFP_LITERAL=3 };
struct mfp_span { const unsigned char *bytes; size_t length; };
struct mfp_arg { unsigned kind; uint32_t number; struct mfp_span span; };
struct mfp_node { unsigned kind, index; size_t offset, length; };
struct mfp_program {
    unsigned char source[32]; size_t source_length, count;
    struct mfp_node nodes[32]; unsigned required[4];
};
struct mfp_message { unsigned char bytes[128]; size_t length; };
struct mfp_arg mfp_number(uint32_t value);
struct mfp_arg mfp_bytes(struct mfp_span value);
#define MFP_ARG(value) _Generic((value),uint32_t:mfp_number,struct mfp_span:mfp_bytes)(value)
int mfp_parse(struct mfp_span source, struct mfp_program *output);
int mfp_prepare(const struct mfp_program *program, const struct mfp_arg *args, size_t count, struct mfp_message *output);
int mfp_render(const struct mfp_program *program, const struct mfp_arg *args, size_t count, struct mp_sink *sink);
enum { MFP_WRITE_OK, MFP_WRITE_AGAIN, MFP_WRITE_ERROR };
struct mfp_write_result { size_t accepted; unsigned status; };
typedef struct mfp_write_result (*mfp_writer_fn)(void *context, const unsigned char *bytes, size_t length);
enum { MFP_EMPTY, MFP_ACTIVE, MFP_DONE, MFP_FAILED, MFP_STOPPED };
/* Initially zero. Serialized live objects; see FORMAT.md, especially callback lifetime. */
struct mfp_stream { struct mfp_message message; size_t offset; unsigned state, uncertain; int failure; };
int mfp_start(struct mfp_stream *stream, const struct mfp_message *message);
int mfp_step(struct mfp_stream *stream, mfp_writer_fn writer, void *context);
int mfp_cancel(struct mfp_stream *stream);
#endif
