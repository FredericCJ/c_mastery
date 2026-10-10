static _Thread_local int slot = 9;
__attribute__((visibility("default"))) int mi_tls_next(void) { return ++slot; }
