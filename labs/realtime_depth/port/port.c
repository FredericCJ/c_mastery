#include "port.h"
enum cmx_result sr_init(SRPort *s, cmx_kernel *k, unsigned task,
                       uint32_t quota, uint32_t period) {
  (void)s; (void)k; (void)task; (void)quota; (void)period;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result sr_tick(SRPort *s, uint32_t boundary) {
  (void)s; (void)boundary; return CMX_UNIMPLEMENTED;
}
enum cmx_result sr_close(SRPort *s) {(void)s; return CMX_UNIMPLEMENTED;}
enum cmx_result sr_detach(SRPort *s) {(void)s; return CMX_UNIMPLEMENTED;}
