#ifndef CMASTERY_DOD_DICTIONARY_H
#define CMASTERY_DOD_DICTIONARY_H
#include "dod.h"
typedef struct {unsigned char bytes[2];} DHKey;
typedef struct {void *context;uint32_t (*hash)(void *,DHKey);int (*equal)(void *,DHKey,DHKey);} DHDictionary;
typedef struct {unsigned bitmap,count,children[4];DHKey keys[8];int values[8];} DHMapNode;
typedef struct {DHMapNode nodes[513];unsigned used,fail_at,attempts;DHDictionary dictionary;} DHMap;
typedef struct {unsigned visited,comparisons,clones;} DHMetrics;
int d_map_init(DHMap *,DHDictionary);
int d_map_update(DHMap *,unsigned root,DHKey,int value,unsigned *out,DHMetrics *);
int d_map_delete(DHMap *,unsigned root,DHKey,unsigned *out,DHMetrics *);
int d_map_get(const DHMap *,unsigned root,DHKey,int *out,DHMetrics *);
#endif
