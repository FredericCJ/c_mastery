#ifndef FB_BITS_H
#define FB_BITS_H
#include <stddef.h>
#include <stdint.h>
typedef uint_least32_t FbWord;
typedef enum {FB_OK,FB_INVALID,FB_RANGE,FB_SHORT,FB_UNSUPPORTED,FB_CONFLICT,FB_SKIPPED,FB_UNFINISHED} FbStatus;
typedef enum {FB_LEFT,FB_RIGHT,FB_ROTATE_LEFT} FbShift;
typedef enum {FB_SEQUENTIAL,FB_SIMULTANEOUS} FbPatch;
typedef enum {FB_BIG,FB_LITTLE} FbOrder;
typedef enum {FB_REJECT_UNKNOWN,FB_SKIP_UNKNOWN} FbUnknown;
typedef struct {unsigned low,high;FbWord value;} FbEdit;
typedef struct {unsigned population,width,leading;} FbStats;
typedef struct {const unsigned char *data;size_t size;} FbSpan;
typedef struct {FbWord value;unsigned flags;size_t consumed;} FbRecord;
typedef struct {unsigned version;size_t size;FbWord features,max_items;} FbPlugin;
typedef struct {unsigned char state;unsigned reads,writes;unsigned read_clear;} FbRegister;
FbStatus fb_shift(unsigned width,FbWord input,int count,FbShift policy,FbWord *out);
FbStatus fb_extract(unsigned width,FbWord input,unsigned low,unsigned high,FbWord *out);
FbStatus fb_insert(unsigned width,FbWord input,FbEdit edit,FbWord *out);
FbStatus fb_patch(unsigned width,FbWord input,FbEdit first,FbEdit second,FbPatch policy,FbWord *out);
FbStatus fb_stats(unsigned width,FbWord input,FbStats *out);
FbStatus fb_encode(FbWord value,FbOrder order,unsigned char *out,size_t capacity);
FbStatus fb_decode(FbSpan input,FbOrder order,FbWord *out);
FbStatus fb_decode_two(FbSpan first,FbSpan second,FbOrder order,FbWord *out);
FbStatus fb_record(FbSpan input,FbUnknown policy,FbRecord *out);
FbStatus fb_negotiate(FbSpan input,FbPlugin *out);
/* Original teaching register: low4 ordinary, high4 W1C; read_clear optionally
 * clears high4 after returning old state. No volatile/MMIO or concurrency claim. */
FbStatus fb_register_init(FbRegister *reg,unsigned state,unsigned read_clear);
unsigned fb_register_read(FbRegister *reg);
void fb_register_write(FbRegister *reg,unsigned value);
FbStatus fb_register_update(FbRegister *reg,unsigned low_value,unsigned clear_status);
#endif
