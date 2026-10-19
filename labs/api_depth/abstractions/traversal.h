#ifndef CMASTERY_ABSTRACTION_TRAVERSAL_H
#define CMASTERY_ABSTRACTION_TRAVERSAL_H
#include "ownership.h"
typedef enum { AT_ITEM,AT_END,AT_STOP,AT_STALE,AT_INVALID,AT_FULL,AT_DENIED,AT_CANCELLED,AT_UNIMPLEMENTED } AtResult;
typedef struct { FView view;const AcEpochOwner *owner;size_t position;unsigned limit;AtResult terminal; } AtCursor;
AtResult at_begin(const AcEpochOwner *,unsigned limit,AtCursor *);
AtResult at_next(AtCursor *,unsigned char *);
typedef enum { AT_CONTINUE,AT_STOP_NOW,AT_REMOVE } AtAction;
typedef AtAction (*AtVisitor)(void *,unsigned char);
typedef struct { unsigned depth,max_depth,visits; } AtPush;
AtResult at_push(AtPush *,const AcEpochOwner *,unsigned limit,AtVisitor,void *);
typedef struct { AtCursor cursor;AtVisitor visitor;void *context;unsigned active,running;AtResult terminal; } AtDeferred;
AtResult at_defer(AtDeferred *,const AcEpochOwner *,unsigned limit,AtVisitor,void *);
AtResult at_pump(AtDeferred *,unsigned budget);
AtResult at_cancel(AtDeferred *);
typedef struct { unsigned char value;unsigned left,right; } AtNode;
AtResult at_recursive(const AtNode *,size_t count,unsigned root,unsigned depth_limit,unsigned char *out,size_t capacity,size_t *written);
AtResult at_continuations(const AtNode *,size_t count,unsigned root,unsigned depth_limit,unsigned char *out,size_t capacity,size_t *written);
#endif
