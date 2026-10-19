#ifndef CMASTERY_DOD_EXT_H
#define CMASTERY_DOD_EXT_H
#include "dod.h"
#define D_ROW_FIELDS(F) F(int32_t,x) F(int32_t,velocity) F(unsigned,cold)
#define D_FIELD(type,name) type name;
typedef struct {D_ROW_FIELDS(D_FIELD)} DRow;
#undef D_FIELD
#define D_FIELD(type,name) type name[8];
typedef struct {D_ROW_FIELDS(D_FIELD) size_t count;} DColumns8;
#undef D_FIELD
typedef struct {int32_t x[3],velocity[3];unsigned cold[3];} DTile;
typedef struct {DTile tiles[3];size_t count;unsigned has_cold;} DTiles;
int d_rows_to_columns(const DRow *,size_t,DColumns8 *);
int d_columns_to_rows(const DColumns8 *,DRow *,size_t);
int d_rows_step(DRow *,size_t,int32_t);
int d_columns8_step(DColumns8 *,int32_t);
int d_rows_step_many(DRow *,size_t,int32_t);
int d_columns_step_many(int32_t *,const int32_t *,size_t,int32_t);
int d_rows_to_arrays(const DRow *,size_t,int32_t *,int32_t *,unsigned *,size_t);
int d_arrays_to_rows(const int32_t *,const int32_t *,const unsigned *,size_t,DRow *,size_t);
int d_rows_to_tiles(const DRow *,size_t,unsigned,DTiles *);
int d_tiles_to_rows(const DTiles *,DRow *,size_t);
uint32_t d_swar16_add(uint32_t,uint32_t);

typedef void (*DDestroy)(void *,void *);
typedef struct {DSparse pages[2];void *resource[2][4];DDestroy destroy;void *context;} DSegment;
typedef struct {const DSegment *owner;unsigned page;DHandle handle;} DSegmentHandle;
void d_segment_init(DSegment *,DDestroy,void *);
int d_segment_add(DSegment *,int,void *,DSegmentHandle *);
int d_segment_get(const DSegment *,DSegmentHandle,int *);
int d_segment_borrow(const DSegment *,DSegmentHandle,const int **);
int d_segment_remove(DSegment *,DSegmentHandle);

typedef struct {DEcs ecs;unsigned version;} DQueryWorld;
typedef struct {const DQueryWorld *owner;unsigned version,required,excluded,ids[4],count;} DQuery;
void d_query_world_init(DQueryWorld *);
int d_query_set(DQueryWorld *,unsigned,unsigned,int,int);
int d_query_get(const DQueryWorld *,unsigned required,unsigned excluded,DQuery *);
#endif
