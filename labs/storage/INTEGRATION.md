# Block and VM adapters

The store owns page layout and recovery. The adapter owns transport, transfer
completion and the meaning of a successful flush. Neither layer reaches into the
other's internal data structures. Supply exactly 67 or more exclusive image pages.

```c
/* Kernel wrappers implement these signatures using the actual kernel ABI.
   They copy exactly 4096 bytes and return zero only on complete success. */
extern int user_block_read(unsigned page, void *out);
extern int user_block_write(unsigned page, const void *in);
extern int user_block_flush(void);
static int read_adapter(void *ctx, uint32_t page, void *out) {
    (void)ctx; return user_block_read(page, out);
}
static int write_adapter(void *ctx, uint32_t page, const void *in) {
    (void)ctx; return user_block_write(page, in);
}
static int flush_adapter(void *ctx) {
    (void)ctx; return user_block_flush();
}
/* store_io io = {NULL, read_adapter, write_adapter, flush_adapter, 67}; */
```

The exported kernel ABI defines syscall numbers/registers and buffer validation.
No pointer is trusted merely because it came from the store. A completed emulated
device request is not persistent until the declared device flush succeeds. The
kernel grants a process an exclusive slice; the store adds no system-wide lock.
Validate page arithmetic in both layers. A physical drive or flash device needs
its own documented flush/erase and failure model.

A VM callback may offer only `put vm-result` and `get vm-result`: encode its int32
argument as four explicit bytes, begin a transaction, write, commit and map the
result to a VM error. Read checks the value length before decoding. A callback
cannot return successful execution after `STORE_INDETERMINATE`. There is no native
FFI and no arbitrary filename or pointer supplied by bytecode. VM host-call budget
and storage transaction capacity remain separate bounds.

The component tests establish the callback contract against memory and Linux file
adapters. A full integration submission must run actual bytecode, persist a result,
restart, recover and read it, then reproduce a lost-response case. The kernel
variant adds its real syscall transcript and boot/image identity. Until executed,
those cross-family results stay `not-run` rather than inheriting component credit.
