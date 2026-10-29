#ifndef CMASTERY_ABSTRACTION_TRAVERSAL_H
#define CMASTERY_ABSTRACTION_TRAVERSAL_H
#include "ownership.h"
enum at_result { AT_ITEM,AT_END,AT_STOP,AT_STALE,AT_INVALID,AT_FULL,AT_DENIED,AT_CANCELLED,AT_UNIMPLEMENTED };
struct at_cursor { struct f_view view;const struct ac_epoch_owner *owner;size_t position;unsigned limit;enum at_result terminal; };
enum at_result at_begin(const struct ac_epoch_owner *,unsigned limit,struct at_cursor *);
enum at_result at_next(struct at_cursor *,unsigned char *);
enum at_action { AT_CONTINUE,AT_STOP_NOW,AT_REMOVE };
typedef enum at_action (*at_visitor_fn)(void *,unsigned char);
struct at_push { unsigned depth,max_depth,visits; };
enum at_result at_push(struct at_push *,const struct ac_epoch_owner *,unsigned limit,at_visitor_fn,void *);
struct at_deferred { struct at_cursor cursor;at_visitor_fn visitor;void *context;unsigned active,running;enum at_result terminal; };
enum at_result at_defer(struct at_deferred *,const struct ac_epoch_owner *,unsigned limit,at_visitor_fn,void *);
enum at_result at_pump(struct at_deferred *,unsigned budget);
enum at_result at_cancel(struct at_deferred *);
struct at_node { unsigned char value;unsigned left,right; };
enum at_result at_recursive(const struct at_node *,size_t count,unsigned root,unsigned depth_limit,unsigned char *out,size_t capacity,size_t *written);
enum at_result at_continuations(const struct at_node *,size_t count,unsigned root,unsigned depth_limit,unsigned char *out,size_t capacity,size_t *written);
#endif
