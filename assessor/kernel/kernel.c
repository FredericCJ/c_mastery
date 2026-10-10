#include "cmk_abi.h"
#include "cmk_contract.h"
#define PACKED __attribute__((packed))
#define ALIGN(n) __attribute__((aligned(n)))
#define PTE_NX (UINT64_C(1) << 63)
#define TASKS 6
void *memset(void *d, int c, size_t n) {
  unsigned char *p = d;
  while (n--)
    *p++ = (unsigned char)c;
  return d;
}
void *memcpy(void *d, const void *s, size_t n) {
  unsigned char *p = d;
  const unsigned char *q = s;
  while (n--)
    *p++ = *q++;
  return d;
}
void *memmove(void *d, const void *s, size_t n) {
  unsigned char *p = d;
  const unsigned char *q = s;
  /* Fixed x86-64 flat-address profile: compare integer addresses, not C
     pointers that may belong to different allocated objects. */
  if ((uintptr_t)p < (uintptr_t)q)
    while (n--)
      *p++ = *q++;
  else
    while (n) {
      n--;
      p[n] = q[n];
    }
  return d;
}
int memcmp(const void *a, const void *b, size_t n) {
  const unsigned char *p = a, *q = b;
  while (n--) {
    if (*p != *q)
      return *p < *q ? -1 : 1;
    p++;
    q++;
  }
  return 0;
}
static inline void outb(uint16_t p, uint8_t v) {
  __asm__ volatile("outb %0,%1" ::"a"(v), "Nd"(p));
}
static inline uint8_t inb(uint16_t p) {
  uint8_t v;
  __asm__ volatile("inb %1,%0" : "=a"(v) : "Nd"(p));
  return v;
}
static inline void outl(uint16_t p, uint32_t v) {
  __asm__ volatile("outl %0,%1" ::"a"(v), "Nd"(p));
}
static inline uint32_t inl(uint16_t p) {
  uint32_t v;
  __asm__ volatile("inl %1,%0" : "=a"(v) : "Nd"(p));
  return v;
}
static void barrier(void) { __asm__ volatile("mfence" ::: "memory"); }
static void serial_init(void) {
  outb(0x3f9, 0);
  outb(0x3fb, 0x80);
  outb(0x3f8, 1);
  outb(0x3f9, 0);
  outb(0x3fb, 3);
  outb(0x3fa, 0xc7);
  outb(0x3fc, 0xb);
}
static void put(char c) {
  for (unsigned n = 0; n < 1000000; n++)
    if (inb(0x3fd) & 32) {
      outb(0x3f8, (unsigned char)c);
      return;
    }
}
static void say(const char *s) {
  while (*s)
    put(*s++);
}
static void hex(uint64_t n) {
  say("0x");
  for (int i = 60; i >= 0; i -= 4)
    put("0123456789abcdef"[(n >> i) & 15]);
}
static __attribute__((noreturn)) void stop(int success) {
  outl(0xf4, success ? 0x10 : 0x11);
  for (;;)
    __asm__ volatile("cli; hlt");
}
static __attribute__((noreturn)) void panic(const char *s) {
  say("FAIL kernel: ");
  say(s);
  say("\n");
  stop(0);
}
extern uint64_t boot_pdpt[], boot_pd[];
extern unsigned char kernel_end[];
extern void *isr_table[];
extern void isr32(void), isr128(void), enter_frame(void *);
struct frame {
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rdx, rcx, rbx,
      rax, vector, error, rip, cs, flags, rsp, ss;
};
struct task {
  uint64_t pml4[512], pdpt[512], pd[512], pt[512];
  unsigned char stack[8192];
  struct frame *frame;
  int alive, status, reaped, handle_open;
  unsigned generation;
  const char *name;
} ALIGN(4096);
static struct task tasks[TASKS];
static unsigned task_count, current;
static uint64_t ticks, preemptions;
static unsigned faults;
static int boot_mode;
static struct cmk_frames frames;
static const unsigned char *archive;
static size_t archive_size;
struct tss64 {
  uint32_t reserved;
  uint64_t rsp0, rsp1, rsp2, r2, ist[7], r3;
  uint16_t r4, iomap;
} PACKED;
static struct tss64 tss;
static unsigned char fault_stack[8192] ALIGN(16);
static uint64_t gdt[7];
struct idt_gate {
  uint16_t low, selector;
  uint8_t ist, type;
  uint16_t middle;
  uint32_t high, zero;
} PACKED;
static struct idt_gate idt[256];
struct desc_ptr {
  uint16_t limit;
  uint64_t base;
} PACKED;
static void gate(unsigned n, void (*fn)(void), unsigned dpl, unsigned ist) {
  uint64_t p = (uint64_t)(uintptr_t)fn;
  idt[n] = (struct idt_gate){p & 65535,
                             8,
                             (uint8_t)ist,
                             (uint8_t)(0x8e | (dpl << 5)),
                             (p >> 16) & 65535,
                             (uint32_t)(p >> 32),
                             0};
}
static void tables(void) {
  gdt[0] = 0;
  gdt[1] = UINT64_C(0x00af9a000000ffff);
  gdt[2] = UINT64_C(0x00cf92000000ffff);
  gdt[3] = UINT64_C(0x00affa000000ffff);
  gdt[4] = UINT64_C(0x00cff2000000ffff);
  uint64_t p = (uint64_t)(uintptr_t)&tss, lim = sizeof(tss) - 1;
  tss.iomap = sizeof(tss);
  tss.ist[0] = (uint64_t)(uintptr_t)(fault_stack + sizeof fault_stack);
  gdt[5] = (lim & 65535) | ((p & 0xffffff) << 16) | (UINT64_C(0x89) << 40) |
           ((lim & 0xf0000) << 32) | ((p & UINT64_C(0xff000000)) << 32);
  gdt[6] = p >> 32;
  struct desc_ptr gd = {sizeof(gdt) - 1, (uint64_t)(uintptr_t)gdt};
  __asm__ volatile("lgdt %0" ::"m"(gd));
  uint16_t sel = 40;
  __asm__ volatile("ltr %0" ::"r"(sel));
  for (unsigned i = 0; i < 32; i++)
    gate(i, (void (*)(void))isr_table[i], 0, i == 8 ? 1 : 0);
  gate(32, isr32, 0, 0);
  gate(128, isr128, 3, 0);
  struct desc_ptr di = {sizeof(idt) - 1, (uint64_t)(uintptr_t)idt};
  __asm__ volatile("lidt %0" ::"m"(di));
  /* Fixed PC profile: legacy PIC/PIT, timer IRQ only; no APIC/SMP route. */
  outb(0x20, 0x11);
  outb(0xa0, 0x11);
  outb(0x21, 32);
  outb(0xa1, 40);
  outb(0x21, 4);
  outb(0xa1, 2);
  outb(0x21, 1);
  outb(0xa1, 1);
  outb(0x21, 0xfe);
  outb(0xa1, 0xff);
  outb(0x43, 0x36);
  outb(0x40, 11932 & 255);
  outb(0x40, 11932 >> 8);
}
static uint32_t take(void) {
  uint32_t p = cmk_frame_take(&frames);
  if (!p)
    panic("frame exhaustion");
  memset((void *)(uintptr_t)p, 0, 4096);
  return p;
}
static int map_user(struct task *t, unsigned index, unsigned writable,
                    unsigned executable) {
  if (index >= 512 || t->pt[index])
    return -1;
  uint32_t p = cmk_frame_take(&frames);
  if (!p)
    return -1;
  memset((void *)(uintptr_t)p, 0, 4096);
  t->pt[index] =
      (uint64_t)p | 5 | (writable ? 2 : 0) | (executable ? 0 : PTE_NX);
  return 0;
}
static void *physical(struct task *t, uint64_t address) {
  uint64_t pte = t->pt[(address - CMK_USER_BASE) / 4096];
  return (void *)(uintptr_t)((pte & UINT64_C(0x000ffffffffff000)) +
                             (address & 4095));
}
static int user_ok(struct task *t, uint64_t p, uint64_t n, int write) {
  if (!cmk_user_range(p, n))
    return 0;
  while (n) {
    unsigned index = (unsigned)((p - CMK_USER_BASE) / 4096);
    uint64_t e = t->pt[index];
    if ((e & 5) != 5 || (write && !(e & 2)))
      return 0;
    uint64_t step = 4096 - (p & 4095);
    if (step > n)
      step = n;
    p += step;
    n -= step;
  }
  return 1;
}
static int load_task(const char *name, int disk) {
  if (task_count == TASKS)
    return -1;
  const unsigned char *data;
  size_t size;
  struct cmk_elf elf;
  if (cmk_archive_find(archive, archive_size, name, &data, &size) ||
      cmk_elf_parse(data, size, &elf))
    return -1;
  struct task *t = &tasks[task_count];
  memset(t, 0, sizeof *t);
  t->name = name;
  t->generation = 1;
  t->handle_open = disk;
  t->pml4[0] = (uint64_t)(uintptr_t)t->pdpt | 7;
  for (unsigned i = 0; i < 4; i++)
    t->pdpt[i] = boot_pdpt[i];
  t->pdpt[1] = (uint64_t)(uintptr_t)t->pd | 7;
  t->pd[0] = (uint64_t)(uintptr_t)t->pt | 7;
  for (unsigned i = 0; i < elf.count; i++) {
    struct cmk_segment *s = &elf.segments[i];
    uint64_t end = s->address + s->memsz;
    for (uint64_t a = s->address & ~UINT64_C(4095); a < end; a += 4096)
      if (map_user(t, (unsigned)((a - CMK_USER_BASE) / 4096), s->flags & 2,
                   s->flags & 1))
        panic("ELF mapping");
    for (uint64_t j = 0; j < s->filesz; j++)
      *(unsigned char *)physical(t, s->address + j) = data[s->offset + j];
  }
  for (unsigned i = 480; i < 512; i++)
    if (map_user(t, i, 1, 0))
      panic("stack mapping");
  t->frame = (struct frame *)(void *)(t->stack + sizeof t->stack -
                                      sizeof(struct frame));
  memset(t->frame, 0, sizeof *t->frame);
  t->frame->rip = elf.entry;
  t->frame->cs = 27;
  t->frame->ss = 35;
  t->frame->flags = 0x202;
  t->frame->rsp = CMK_USER_LIMIT - 8;
  t->alive = 1;
  task_count++;
  return 0;
}
static void activate(unsigned next) {
  current = next;
  tss.rsp0 =
      (uint64_t)(uintptr_t)(tasks[next].stack + sizeof tasks[next].stack);
  uint64_t cr3 = (uint64_t)(uintptr_t)tasks[next].pml4;
  __asm__ volatile("mov %0,%%cr3" ::"r"(cr3) : "memory");
}
static void reap(void) {
  for (unsigned j = 0; j < task_count; j++) {
    struct task *t = &tasks[j];
    if (t->alive || t->reaped)
      continue;
    for (unsigned i = 0; i < 512; i++)
      if (t->pt[i] & 1) {
        uint32_t p = (uint32_t)(t->pt[i] & UINT64_C(0xfffff000));
        if (cmk_frame_put(&frames, p))
          panic("reap ownership");
        t->pt[i] = 0;
      }
    t->reaped = 1;
  }
}
/* Modern virtio PCI, one split queue and one request. Static DMA storage
 * remains owned by the device after a timeout: mark device failed, never
 * recycle it. */
struct vdesc {
  uint64_t addr;
  uint32_t len;
  uint16_t flags, next;
} PACKED;
struct vavail {
  volatile uint16_t flags, idx, ring[8], used_event;
} PACKED;
struct vused {
  volatile uint16_t flags, idx;
  struct {
    volatile uint32_t id, len;
  } ring[8];
  volatile uint16_t avail_event;
} PACKED;
static struct vdesc vd[8] ALIGN(4096);
static struct vavail va ALIGN(4096);
static struct vused vu ALIGN(4096);
static struct {
  uint32_t type, reserved;
  uint64_t sector;
} request ALIGN(16);
static unsigned char bounce[4096] ALIGN(4096);
static volatile unsigned char request_status;
static volatile unsigned char *common_cfg, *device_cfg;
static volatile uint16_t *notify;
static uint32_t disk_pages;
static uint16_t used_idx;
static int disk_ready;
static uint32_t pci_read(unsigned devfn, unsigned off) {
  outl(0xcf8, 0x80000000u | (devfn << 8) | (off & 252));
  return inl(0xcfc);
}
static void pci_write(unsigned devfn, unsigned off, uint32_t v) {
  outl(0xcf8, 0x80000000u | (devfn << 8) | (off & 252));
  outl(0xcfc, v);
}
static uint8_t pci_byte(unsigned d, unsigned o) {
  return (uint8_t)(pci_read(d, o) >> (8 * (o & 3)));
}
#define R16(p, o) (*(volatile uint16_t *)((p) + (o)))
#define R32(p, o) (*(volatile uint32_t *)((p) + (o)))
#define R64(p, o) (*(volatile uint64_t *)((p) + (o)))
static void virtio_init(void) {
  unsigned d;
  for (d = 0; d < 256; d++)
    if (pci_read(d, 0) == 0x10421af4)
      break;
  if (d == 256)
    panic("modern virtio-blk absent");
  if (!(pci_read(d, 4) & 0x100000))
    panic("PCI capability list absent");
  volatile unsigned char *notify_base = 0;
  uint32_t mult = 0;
  unsigned cap = pci_byte(d, 0x34) & ~3u;
  unsigned seen = 0;
  uint32_t notify_size = 0;
  while (cap && seen++ < 48) {
    if (cap < 0x40 || cap > 0xec)
      panic("PCI capability bounds");
    unsigned next = pci_byte(d, cap + 1) & ~3u;
    if (pci_byte(d, cap) == 9) {
      unsigned len = pci_byte(d, cap + 2), kind = pci_byte(d, cap + 3),
               bar = pci_byte(d, cap + 4);
      if (kind < 1 || kind > 4) {
        cap = next;
        continue;
      }
      if (len < 16 || bar > 5 || cap + len > 256)
        panic("virtio capability extent");
      uint32_t lo = pci_read(d, 0x10 + bar * 4);
      if (lo & 1)
        panic("virtio I/O BAR");
      uint64_t base = lo & ~UINT32_C(15);
      if ((lo & 6) == 4) {
        if (bar == 5)
          panic("64-bit BAR index");
        base |= (uint64_t)pci_read(d, 0x14 + bar * 4) << 32;
      }
      uint32_t off = pci_read(d, cap + 8), size = pci_read(d, cap + 12);
      if (!size || base < 0x80000000 || base + off >= UINT64_C(0x100000000) ||
          size > UINT64_C(0x100000000) - base - off)
        panic("virtio BAR scope");
      volatile unsigned char *p =
          (volatile unsigned char *)(uintptr_t)(base + off);
      if (kind == 1) {
        if (size < 56)
          panic("common length");
        common_cfg = p;
      }
      if (kind == 2) {
        if (len < 20)
          panic("notify length");
        notify_base = p;
        notify_size = size;
        mult = pci_read(d, cap + 16);
      }
      if (kind == 4) {
        if (size < 8)
          panic("device length");
        device_cfg = p;
      }
    }
    cap = next;
  }
  if (cap || !common_cfg || !device_cfg || !notify_base)
    panic("virtio capability contract");
  pci_write(d, 4, (pci_read(d, 4) & 65535) | 6);
  common_cfg[20] = 0;
  barrier();
  if (common_cfg[20])
    panic("virtio reset");
  common_cfg[20] = 3;
  R32(common_cfg, 0) = 1;
  if (!(R32(common_cfg, 4) & 1))
    panic("VERSION_1 absent");
  R32(common_cfg, 0) = 0;
  if (!(R32(common_cfg, 4) & (1u << 9)))
    panic("FLUSH absent");
  R32(common_cfg, 8) = 0;
  R32(common_cfg, 12) = 1u << 9;
  R32(common_cfg, 8) = 1;
  R32(common_cfg, 12) = 1;
  common_cfg[20] = 11;
  if (!(common_cfg[20] & 8))
    panic("feature negotiation");
  R16(common_cfg, 22) = 0;
  if (R16(common_cfg, 24) < 8)
    panic("queue too small");
  R16(common_cfg, 24) = 8;
  R16(common_cfg, 26) = 65535;
  R64(common_cfg, 32) = (uint64_t)(uintptr_t)vd;
  R64(common_cfg, 40) = (uint64_t)(uintptr_t)&va;
  R64(common_cfg, 48) = (uint64_t)(uintptr_t)&vu;
  uint64_t notify_offset = (uint64_t)R16(common_cfg, 30) * mult;
  if (notify_size < 2 || notify_offset > notify_size - 2)
    panic("notify bounds");
  notify = (volatile uint16_t *)(notify_base + notify_offset);
  R16(common_cfg, 28) = 1;
  common_cfg[20] = 15;
  uint64_t sectors = R64(device_cfg, 0);
  if (sectors < 16 || sectors / 8 > UINT32_MAX)
    panic("disk size contract");
  disk_pages = (uint32_t)(sectors / 8);
  disk_ready = 1;
  say("PASS virtio-modern-flush\n");
}
static int block(unsigned type, uint32_t page) {
  if (!disk_ready || (type != 4 && page >= disk_pages))
    return -1;
  request.type = type;
  request.reserved = 0;
  request.sector = (uint64_t)page * 8;
  request_status = 255;
  vd[0] = (struct vdesc){(uint64_t)(uintptr_t)&request, sizeof request, 1,
                         (uint16_t)(type == 4 ? 2 : 1)};
  vd[1] = (struct vdesc){(uint64_t)(uintptr_t)bounce, 4096,
                         (uint16_t)(1 | (type == 0 ? 2 : 0)), 2};
  vd[2] = (struct vdesc){(uint64_t)(uintptr_t)&request_status, 1, 2, 0};
  va.ring[va.idx % 8] = 0;
  barrier();
  va.idx++;
  barrier();
  *notify = 0;
  unsigned poll;
  for (poll = 0; poll < 20000000 && vu.idx == used_idx; poll++)
    __asm__ volatile("pause" ::: "memory");
  barrier();
  if (vu.idx == used_idx) {
    disk_ready = 0;
    common_cfg[20] |= 128;
    return -1;
  }
  unsigned slot = used_idx % 8;
  if (vu.ring[slot].id != 0) {
    disk_ready = 0;
    return -1;
  }
  used_idx++;
  if (vu.idx != used_idx || request_status != 0)
    return -1;
  return 0;
}
static void complete(void) {
  if (boot_mode != 2) {
    if (faults != 1 || preemptions < 2 || ticks < 6)
      panic("isolation/preemption evidence missing");
    for (unsigned i = 0; i < task_count; i++)
      if (i != 3 && tasks[i].status)
        panic("user test failed");
    say("PASS isolation-preemption\nPASS kernel-suite\n");
    stop(1);
  }
  say("shell exited\n");
  stop(1);
}
static struct frame *schedule(void) {
  unsigned old = current;
  for (unsigned step = 1; step <= task_count; step++) {
    unsigned next = (old + step) % task_count;
    if (tasks[next].alive) {
      activate(next);
      reap();
      return tasks[next].frame;
    }
  }
  complete();
  return 0;
}
static int64_t syscall(struct frame *f) {
  struct task *t = &tasks[current];
  uint64_t a = f->rdi, b = f->rsi, c = f->rdx;
  switch (f->rax) {
  case CMK_WRITE:
    if (b > 512 || !user_ok(t, a, b, 0))
      return -1;
    for (uint64_t i = 0; i < b; i++)
      put(((const char *)(uintptr_t)a)[i]);
    return (int64_t)b;
  case CMK_READ:
    if (!user_ok(t, a, 1, 1))
      return -1;
    if (!(inb(0x3fd) & 1))
      return 0;
    *(unsigned char *)(uintptr_t)a = inb(0x3f8);
    return 1;
  case CMK_EXIT:
    t->status = (int)a;
    t->alive = 0;
    t->handle_open = 0;
    t->generation++;
    return 0;
  case CMK_YIELD:
    return 0;
  case CMK_TICKS:
    return (int64_t)ticks;
  case CMK_PID:
    return (int64_t)current;
  case CMK_BOOT_MODE:
    return boot_mode;
  case CMK_ALLOC: {
    if (!a || a > 16)
      return -1;
    unsigned first;
    for (first = 256; first + a <= 479; first++) {
      unsigned j;
      for (j = 0; j < a; j++)
        if (t->pt[first + j])
          break;
      if (j == a)
        break;
    }
    if (first + a > 479)
      return -1;
    unsigned j;
    for (j = 0; j < a; j++)
      if (map_user(t, first + j, 1, 0))
        break;
    if (j != a) {
      while (j) {
        j--;
        cmk_frame_put(&frames,
                      (uint32_t)(t->pt[first + j] & UINT64_C(0xfffff000)));
        t->pt[first + j] = 0;
      }
      return -1;
    }
    return (int64_t)(CMK_USER_BASE + (uint64_t)first * 4096);
  }
  case CMK_BLOCK_COUNT:
    if (!cmk_handle_valid((uint32_t)a, 0, t->generation, t->handle_open) ||
        a > UINT32_MAX)
      return -1;
    return disk_pages;
  case CMK_CLOSE:
    if (!cmk_handle_valid((uint32_t)a, 0, t->generation, t->handle_open) ||
        a > UINT32_MAX)
      return -1;
    t->handle_open = 0;
    t->generation++;
    return 0;
  case CMK_BLOCK_READ:
  case CMK_BLOCK_WRITE:
  case CMK_BLOCK_FLUSH: {
    if (a > UINT32_MAX ||
        !cmk_handle_valid((uint32_t)a, 0, t->generation, t->handle_open))
      return -1;
    if (f->rax == CMK_BLOCK_FLUSH)
      return block(4, 0);
    if (b >= disk_pages || !user_ok(t, c, 4096, f->rax == CMK_BLOCK_READ))
      return -1;
    if (f->rax == CMK_BLOCK_WRITE)
      memcpy(bounce, (const void *)(uintptr_t)c, 4096);
    int r = block(f->rax == CMK_BLOCK_READ ? 0 : 1, (uint32_t)b);
    if (!r && f->rax == CMK_BLOCK_READ)
      memcpy((void *)(uintptr_t)c, bounce, 4096);
    return r;
  }
  case CMK_BOOT_FILE: {
    char name[24];
    if (!user_ok(t, a, 24, 0) || c > 65536 || !user_ok(t, b, c, 1))
      return -1;
    memcpy(name, (const void *)(uintptr_t)a, 24);
    unsigned terminated = 0;
    for (unsigned i = 0; i < sizeof name; i++)
      if (!name[i])
        terminated = 1;
    if (!terminated)
      return (uint64_t)-1;
    const unsigned char *p;
    size_t n;
    if (cmk_archive_find(archive, archive_size, name, &p, &n) || n > c)
      return -1;
    memcpy((void *)(uintptr_t)b, p, n);
    return (int64_t)n;
  }
  default:
    return -1;
  }
}
struct frame *dispatch(struct frame *f) {
  if ((f->cs & 3) != 3) {
    say("kernel exception ");
    hex(f->vector);
    say(" rip ");
    hex(f->rip);
    say(" error ");
    hex(f->error);
    panic("ring0 trap");
  }
  tasks[current].frame = f;
  if (f->vector == 32) {
    ticks++;
    outb(0x20, 0x20);
    preemptions++;
    return schedule();
  }
  if (f->vector == 128) {
    uint64_t n = f->rax;
    f->rax = (uint64_t)syscall(f);
    if (n == CMK_YIELD || !tasks[current].alive)
      return schedule();
    return f;
  }
  if (f->vector == 14 && current == 3) {
    uint64_t cr2;
    __asm__ volatile("mov %%cr2,%0" : "=r"(cr2));
    if (cr2 != 0x100000 || (f->error & 7) != 7)
      panic("unexpected protection fault");
    faults++;
    say("PASS user-kernel-write-denied\n");
    tasks[current].alive = 0;
    tasks[current].status = -14;
    return schedule();
  }
  say("user fault ");
  hex(f->vector);
  say(" rip ");
  hex(f->rip);
  say("\n");
  tasks[current].status = -(int)f->vector - 1;
  tasks[current].alive = 0;
  tasks[current].handle_open = 0;
  tasks[current].generation++;
  return schedule();
}
void kernel_main(unsigned magic, unsigned info) {
  serial_init();
  say("CMK v1 boot\n");
  if (magic != 0x36d76289)
    panic("multiboot magic");
  struct cmk_boot b;
  const unsigned char *p = (const unsigned char *)(uintptr_t)info;
  uint32_t bytes = *(const uint32_t *)p;
  if (info > 0x8000000 || bytes > 1048576 || bytes > 0x8000000 - info ||
      cmk_boot_parse(p, bytes, &b))
    panic("boot tags");
  if (b.module_end > 0x8000000)
    panic("archive placement");
  archive = (const unsigned char *)(uintptr_t)b.module_start;
  archive_size = b.module_end - b.module_start;
  for (unsigned off = 8; off + 8 <= bytes;) {
    uint32_t type = *(const uint32_t *)(p + off),
             size = *(const uint32_t *)(p + off + 4);
    if (type == 1) {
      const char *s = (const char *)(p + off + 8);
      size_t n = size - 8;
      for (size_t i = 0; i + 6 < n; i++)
        if (!memcmp(s + i, "verify", 6))
          boot_mode = 1;
      for (size_t i = 0; i + 11 < n; i++)
        if (!memcmp(s + i, "interactive", 11))
          boot_mode = 2;
    }
    if (!type)
      break;
    off = (off + size + 7) & ~7u;
  }
  uint32_t reserved = (uint32_t)(uintptr_t)kernel_end;
  if (b.module_end > reserved)
    reserved = b.module_end;
  if (info + bytes > reserved)
    reserved = info + bytes;
  reserved = (reserved + 4095) & ~4095u;
  cmk_frames_init(&frames, &b, reserved);
  uint32_t probe = take();
  if (cmk_frame_put(&frames, probe))
    panic("allocator");
  say("PASS boot-map-frames\n");
  tables();
  virtio_init();
  if (load_task(boot_mode == 2 ? "shell" : "init", 1))
    panic("init ELF");
  if (boot_mode != 2) {
    if (load_task("worker", 0) || load_task("worker", 0) ||
        load_task("fault", 0))
      panic("test ELF");
    const unsigned char *vm;
    size_t n;
    if (!cmk_archive_find(archive, archive_size, "vm", &vm, &n) &&
        load_task("vm", 0))
      panic("VM ELF");
  }
  say("PASS elf-load\n");
  activate(0);
  enter_frame(tasks[0].frame);
  panic("returned from user");
}
