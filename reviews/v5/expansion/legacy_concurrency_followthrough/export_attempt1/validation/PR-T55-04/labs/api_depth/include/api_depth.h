#ifndef CMASTERY_API_DEPTH_H
#define CMASTERY_API_DEPTH_H
#include <stddef.h>
#include <stdint.h>
typedef enum { API_OK, API_INVALID, API_FULL, API_OVERLAP, API_BUSY,
               API_PENDING, API_CLOSED, API_STALE, API_UNIMPLEMENTED } ApiStatus;
typedef struct { unsigned char data[32]; size_t length, capacity; } ApiBuffer;
ApiStatus api_append(ApiBuffer *, const unsigned char *, size_t);
typedef struct { unsigned index; uint32_t generation; } ApiHandle;
typedef struct ApiRegistry ApiRegistry;
typedef void (*ApiCallback)(void *, ApiRegistry *, ApiHandle);
typedef void (*ApiDrop)(void *);
typedef struct {
    ApiCallback callback;
    ApiDrop drop;
    void *context;
    uint32_t generation;
    unsigned active, removing;
} ApiSlot;
struct ApiRegistry { ApiSlot slots[4]; unsigned dispatching, releasing, closing, closed; };
void api_registry_init(ApiRegistry *);
ApiStatus api_add(ApiRegistry *, ApiCallback, void *, ApiDrop, ApiHandle *);
ApiStatus api_remove(ApiRegistry *, ApiHandle);
ApiStatus api_dispatch(ApiRegistry *);
ApiStatus api_destroy(ApiRegistry *);
#endif
