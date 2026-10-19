#ifndef CMASTERY_FORMAT_STREAM_H
#define CMASTERY_FORMAT_STREAM_H
#include "metaprog.h"
enum { MFP_OK, MFP_INVALID, MFP_FORMAT, MFP_TYPE, MFP_FULL, MFP_MORE,
       MFP_AGAIN, MFP_IO, MFP_PROTOCOL, MFP_CANCELLED, MFP_BUSY };
enum { MFP_NUMBER=1, MFP_BYTES=2, MFP_LITERAL=3 };
typedef struct { const unsigned char *bytes; size_t length; } MfpSpan;
typedef struct { unsigned kind; uint32_t number; MfpSpan span; } MfpArg;
typedef struct { unsigned kind, index; size_t offset, length; } MfpNode;
typedef struct {
    unsigned char source[32]; size_t source_length, count;
    MfpNode nodes[32]; unsigned required[4];
} MfpProgram;
typedef struct { unsigned char bytes[128]; size_t length; } MfpMessage;
MfpArg mfp_number(uint32_t value);
MfpArg mfp_bytes(MfpSpan value);
#define MFP_ARG(value) _Generic((value),uint32_t:mfp_number,MfpSpan:mfp_bytes)(value)
int mfp_parse(MfpSpan source, MfpProgram *output);
int mfp_prepare(const MfpProgram *program, const MfpArg *args, size_t count, MfpMessage *output);
int mfp_render(const MfpProgram *program, const MfpArg *args, size_t count, MPSink *sink);
enum { MFP_WRITE_OK, MFP_WRITE_AGAIN, MFP_WRITE_ERROR };
typedef struct { size_t accepted; unsigned status; } MfpWriteResult;
typedef MfpWriteResult (*MfpWriter)(void *context, const unsigned char *bytes, size_t length);
enum { MFP_EMPTY, MFP_ACTIVE, MFP_DONE, MFP_FAILED, MFP_STOPPED };
/* Initially zero. Serialized live objects; see FORMAT.md, especially callback lifetime. */
typedef struct { MfpMessage message; size_t offset; unsigned state, uncertain; int failure; } MfpStream;
int mfp_start(MfpStream *stream, const MfpMessage *message);
int mfp_step(MfpStream *stream, MfpWriter writer, void *context);
int mfp_cancel(MfpStream *stream);
#endif
