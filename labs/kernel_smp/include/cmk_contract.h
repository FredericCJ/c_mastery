#ifndef CMK_CONTRACT_H
#define CMK_CONTRACT_H
#include <stddef.h>
#include <stdint.h>
#define CMK_USER_BASE UINT64_C(0x40000000)
#define CMK_USER_LIMIT UINT64_C(0x40200000)
#define CMK_MAX_SEGMENTS 8
#define CMK_MAX_FRAMES 32768u
struct cmk_segment {
  uint64_t offset, address, filesz, memsz;
  uint32_t flags;
};
struct cmk_elf {
  uint64_t entry;
  unsigned count;
  struct cmk_segment segments[CMK_MAX_SEGMENTS];
};
struct cmk_boot {
  uint32_t module_start, module_end;
  const unsigned char *map;
  uint32_t map_bytes, entry_size;
};
struct cmk_frames {
  unsigned char state[CMK_MAX_FRAMES];
};
int cmk_boot_parse(const void *, size_t, struct cmk_boot *);
int cmk_elf_parse(const void *, size_t, struct cmk_elf *);
int cmk_archive_find(const void *, size_t, const char *, const unsigned char **,
                     size_t *);
void cmk_frames_init(struct cmk_frames *, const struct cmk_boot *,
                     uint32_t reserved_end);
uint32_t cmk_frame_take(struct cmk_frames *);
int cmk_frame_put(struct cmk_frames *, uint32_t);
int cmk_user_range(uint64_t, uint64_t);
uint32_t cmk_handle(unsigned slot, unsigned generation);
int cmk_handle_valid(uint32_t, unsigned slot, unsigned generation, int open);
#endif
