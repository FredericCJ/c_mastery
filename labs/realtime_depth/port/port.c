#include "port.h"
enum cmx_result sr_init(struct sr_port *s, cmx_kernel *k, unsigned task,
                       uint32_t quota, uint32_t period) {
  (void)s; (void)k; (void)task; (void)quota; (void)period;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result sr_tick(struct sr_port *s, uint32_t boundary) {
  (void)s; (void)boundary; return CMX_UNIMPLEMENTED;
}
enum cmx_result sr_close(struct sr_port *s) {(void)s; return CMX_UNIMPLEMENTED;}
enum cmx_result sr_detach(struct sr_port *s) {(void)s; return CMX_UNIMPLEMENTED;}
