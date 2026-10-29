#ifndef CMASTERY_MPL_LOGGING_H
#define CMASTERY_MPL_LOGGING_H
#include <stddef.h>
#include <stdint.h>
enum { MPL_OK, MPL_INVALID, MPL_FULL, MPL_RANGE };
enum { MPL_NUMBER=1, MPL_BYTES=2 };
struct mpl_field { unsigned kind; uint32_t number; const unsigned char *bytes; size_t length; };
struct mpl_site { const char *file; unsigned line; };
struct mpl_value { unsigned kind; uint32_t number; size_t length; unsigned char bytes[8]; };
struct mpl_record { unsigned has_site, line; char file[256]; size_t count; struct mpl_value fields[4]; };
struct mpl_sink { size_t count; struct mpl_record records[4]; };
/* Zero-initialize a fresh sink. Live calls are serialized; see LOGGING.md. */
struct mpl_field mpl_number(uint32_t value);
struct mpl_field mpl_bytes(const unsigned char *bytes,size_t length);
int mpl_write(struct mpl_sink *sink,const struct mpl_site *site,const struct mpl_field *fields,size_t count);
int mpl_log0(struct mpl_sink *sink,struct mpl_site site);
int mpl_log1(struct mpl_sink *sink,struct mpl_site site,struct mpl_field a);
int mpl_log2(struct mpl_sink *sink,struct mpl_site site,struct mpl_field a,struct mpl_field b);
int mpl_log3(struct mpl_sink *sink,struct mpl_site site,struct mpl_field a,struct mpl_field b,struct mpl_field c);
int mpl_log4(struct mpl_sink *sink,struct mpl_site site,struct mpl_field a,struct mpl_field b,struct mpl_field c,struct mpl_field d);
int mpl_log_arity_error(void);
#endif
