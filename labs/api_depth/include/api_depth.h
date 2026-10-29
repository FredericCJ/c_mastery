#ifndef CMASTERY_API_DEPTH_H
#define CMASTERY_API_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum api_status { API_OK, API_INVALID, API_FULL, API_OVERLAP, API_BUSY,
               API_PENDING, API_CLOSED, API_STALE, API_UNIMPLEMENTED };
struct api_buffer { unsigned char data[32]; size_t length, capacity; };
enum api_status api_append(struct api_buffer *, const unsigned char *, size_t);
struct api_handle { unsigned index; uint32_t generation; };
struct api_registry;
typedef void (*api_callback_fn)(void *, struct api_registry *, struct api_handle);
typedef void (*api_drop_fn)(void *);
struct api_slot {
    api_callback_fn callback;
    api_drop_fn drop;
    void *context;
    uint32_t generation;
    unsigned active, removing;
};
struct api_registry { struct api_slot slots[4]; unsigned dispatching, releasing, closing, closed; };
void api_registry_init(struct api_registry *);
enum api_status api_add(struct api_registry *, api_callback_fn, void *, api_drop_fn, struct api_handle *);
enum api_status api_remove(struct api_registry *, struct api_handle);
enum api_status api_dispatch(struct api_registry *);
enum api_status api_destroy(struct api_registry *);
#endif
