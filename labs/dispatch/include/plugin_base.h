#ifndef CMASTERY_DISPATCH_PLUGIN_BASE_H
#define CMASTERY_DISPATCH_PLUGIN_BASE_H
#include <stddef.h>
/* ABI2 common prefix, independent of optional trailing methods. */
struct ds_plugin_base {
  unsigned major,minor;
  size_t size;
  unsigned required_features,provided_features;
  int (*apply)(int,int,int *);
};
unsigned ds_client_features(void);
#endif
