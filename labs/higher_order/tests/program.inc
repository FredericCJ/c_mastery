/* A single finally-encoded program source, specialized to two C carrier APIs. */
#define HO_PROGRAM(P,I,F,OPS) \
typedef struct {OPS ops;int32_t captured;unsigned calls;} P##Context; \
static int P##body(void *raw,I x,I *out){P##Context *c=raw;I value;++c->calls;int s=c->ops.lit(c->captured,&value);if(s)return s;return c->ops.add(x,value,out);} \
static int P##higher(OPS ops,F function,I *out){I one,two,applied;int s=ops.lit(1,&one);if(!s)s=ops.lit(2,&two);if(!s)s=ops.app(function,one,&applied);if(!s)s=ops.add(applied,two,out);return s;}
