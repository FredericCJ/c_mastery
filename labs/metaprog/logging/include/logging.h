#ifndef CMASTERY_MPL_LOGGING_H
#define CMASTERY_MPL_LOGGING_H
#include <stddef.h>
#include <stdint.h>
enum { MPL_OK, MPL_INVALID, MPL_FULL, MPL_RANGE };
enum { MPL_NUMBER=1, MPL_BYTES=2 };
typedef struct { unsigned kind; uint32_t number; const unsigned char *bytes; size_t length; } MplField;
typedef struct { const char *file; unsigned line; } MplSite;
typedef struct { unsigned kind; uint32_t number; size_t length; unsigned char bytes[8]; } MplValue;
typedef struct { unsigned has_site, line; char file[256]; size_t count; MplValue fields[4]; } MplRecord;
typedef struct { size_t count; MplRecord records[4]; } MplSink;
/* Zero-initialize a fresh sink. Live calls are serialized; see LOGGING.md. */
MplField mpl_number(uint32_t value);
MplField mpl_bytes(const unsigned char *bytes,size_t length);
int mpl_write(MplSink *sink,const MplSite *site,const MplField *fields,size_t count);
int mpl_log0(MplSink *sink,MplSite site);
int mpl_log1(MplSink *sink,MplSite site,MplField a);
int mpl_log2(MplSink *sink,MplSite site,MplField a,MplField b);
int mpl_log3(MplSink *sink,MplSite site,MplField a,MplField b,MplField c);
int mpl_log4(MplSink *sink,MplSite site,MplField a,MplField b,MplField c,MplField d);
int mpl_log_arity_error(void);
#endif
