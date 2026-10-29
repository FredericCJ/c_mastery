#include "wire.h"
#include <string.h>
void ew_note(const char *);
void ew_observation(const char *, const uint32_t *, size_t);
#define CHECK(c, m)                  \
    do                               \
    {                                \
        if (!(c))                    \
        {                            \
            ew_note("EW_ASSERT:" m); \
            return 41;               \
        }                            \
    } while (0)
static const unsigned char a[6] = {2, 0, 0, 0, 0, 1}, b[6] = {2, 0, 0, 0, 0, 2},
                           c[6] = {2, 0, 0, 0, 0, 3},
                           group[6] = {255, 255, 255, 255, 255, 255};

static int stream(void)
{
    unsigned char bytes[258], out[256],
        joined[262] = {0, 1, 7, 0, 0, 0, 2, 8, 9};
    size_t n, k, j, used, got;
    unsigned cases = 0;
    struct ew_stream s, old;
    for (n = 0; n <= 256; ++n)
    {
        bytes[0] = (unsigned char)(n >> 8);
        bytes[1] = (unsigned char)n;
        for (k = 0; k < n; ++k)
            bytes[k + 2] = (unsigned char)(k ^ n);
        for (j = 0; j <= n + 2; ++j)
        {
            memset(&s, 0, sizeof s);
            used = 999;
            CHECK(ew_feed(&s, bytes, j, &used) ==
                          (j == n + 2 ? EW_READY : EW_MORE) &&
                      used == j,
                  "split_prefix");
            if (j < n + 2)
                CHECK(ew_feed(&s, bytes + j, n + 2 - j, &used) == EW_READY &&
                          used == n + 2 - j,
                      "split_tail");
            memcpy(&old, &s, sizeof s);
            got = 999;
            memset(out, 0xa5, sizeof out);
            if (n)
                CHECK(ew_take(&s, out, n - 1, &got) == EW_FULL &&
                          memcmp(&s, &old, sizeof s) == 0 && got == 999 &&
                          out[0] == 0xa5,
                      "take_atomic");
            CHECK(ew_take(&s, out, 256, &got) == EW_OK && got == n &&
                      memcmp(out, bytes + 2, n) == 0,
                  "record_value");
            CHECK(ew_end(&s) == EW_EOF, "clean_eof");
            ++cases;
        }
    }
    memset(&s, 0, sizeof s);
    CHECK(ew_feed(&s, joined, 9, &used) == EW_READY && used == 3,
          "retained_suffix");
    CHECK(ew_feed(&s, joined + 3, 6, &used) == EW_READY && used == 0,
          "backpressure");
    CHECK(ew_take(&s, out, 256, &got) == EW_OK && got == 1 && out[0] == 7,
          "copied_record");
    CHECK(ew_feed(&s, joined + 3, 6, &used) == EW_READY && used == 2,
          "zero_record");
    CHECK(ew_take(&s, 0, 0, &got) == EW_OK && got == 0, "zero_take");
    CHECK(ew_feed(&s, joined + 5, 4, &used) == EW_READY && used == 4,
          "third_record");
    CHECK(ew_take(&s, out, 256, &got) == EW_OK && got == 2 && out[1] == 9,
          "third_value");
    memset(&s, 0, sizeof s);
    bytes[0] = 1;
    bytes[1] = 1;
    CHECK(ew_feed(&s, bytes, 2, &used) == EW_POISON && used == 2, "oversize");
    CHECK(ew_feed(&s, 0, 0, &used) == EW_POISON && used == 0,
          "poison_terminal");
    memset(&s, 0, sizeof s);
    CHECK(ew_feed(&s, bytes, 1, &used) == EW_MORE, "partial");
    CHECK(ew_end(&s) == EW_TRUNCATED && ew_end(&s) == EW_POISON, "partial_eof");
    /* Identical C signature and field width do not negotiate endianness. */
    memset(&s, 0, sizeof s);
    bytes[0] = 1;
    bytes[1] = 0;
    CHECK(ew_feed(&s, bytes, 2, &used) == EW_MORE && s.length == 256,
          "peer_disagreement");
    {
        uint32_t v[] = {cases, 3, 256, 2};
        ew_observation("stream", v, 4);
    }
    return 0;
}

static int vlan(void)
{
    unsigned char payload[1500], wire[1522], plain[1518], saved[1522];
    struct ew_vlan v, old;
    struct ed_frame f;
    size_t n, len, got;
    unsigned cases = 0;
    for (n = 0; n < 1500; ++n)
        payload[n] = (unsigned char)(n * 17u + 3u);
    for (n = 0; n <= 1500; ++n)
    {
        CHECK(ew_vlan_build(b, a, 5, 42, (unsigned)n, payload, n, wire,
                            sizeof wire, &len) == EW_OK,
              "vlan_build");
        CHECK(len == 22 + (n < 42 ? 42 : n) && wire[12] == 0x81 &&
                  wire[13] == 0 && wire[14] == 0xa0 && wire[15] == 42,
              "tag_layout");
        CHECK(ew_vlan_parse(wire, len, &v) == EW_OK && v.priority == 5 &&
                  v.vid == 42 && v.frame.payload_length == n &&
                  v.frame.stored_payload == (n < 42 ? 42 : n),
              "vlan_parse");
        CHECK(ew_vlan_pop(wire, len, plain, sizeof plain, &got) == EW_OK &&
                  got == 18 + (n < 46 ? 46 : n),
              "pop_padding");
        CHECK(ed_frame_parse(plain, got, 1, &f) == ED_OK &&
                  f.payload_length == n && memcmp(f.payload, payload, n) == 0,
              "pop_fcs");
        ++cases;
    }
    CHECK(ew_vlan_build(b, a, 0, 1, 0x800, payload, 3, wire, sizeof wire,
                        &len) == EW_OK &&
              len == 64,
          "type_vlan");
    CHECK(ew_vlan_parse(wire, len, &v) == EW_OK && v.frame.payload_length == 42,
          "unknown_inner_length");
    memcpy(&old, &v, sizeof v);
    wire[21] ^= 1;
    CHECK(ew_vlan_parse(wire, len, &v) == EW_POISON &&
              memcmp(&v, &old, sizeof v) == 0,
          "padding_integrity");
    wire[21] ^= 1;
    memcpy(saved, wire, sizeof wire);
    got = 777;
    CHECK(ew_vlan_build(b, a, 0, 4095, 3, payload, 3, wire, sizeof wire,
                        &got) == EW_INVALID &&
              got == 777 && memcmp(saved, wire, sizeof wire) == 0,
          "reserved_vid");
    CHECK(ew_vlan_build(b, a, 0, 1, 0x8100, payload, 3, wire, sizeof wire,
                        &got) == EW_INVALID,
          "nested_tag");
    CHECK(ew_vlan_build(b, a, 8, 1, 3, payload, 3, wire, sizeof wire, &got) ==
              EW_INVALID,
          "pcp_width");
    CHECK(ew_vlan_build(b, a, 0, 1, 3, payload, 3, wire, 63, &got) == EW_FULL &&
              got == 777,
          "capacity_atomic");
    wire[14] |= 0x10;
    CHECK(ew_vlan_parse(wire, len, &v) == EW_INVALID, "cfi_excluded");
    {
        uint32_t v[] = {cases, 64, 1522, 42, 46};
        ew_observation("vlan", v, 5);
    }
    return 0;
}

static int frame(void)
{
    unsigned char payload[3] = {1, 2, 3}, wire[1518];
    size_t n;
    struct ed_frame f, old;
    uint32_t crc0, crc1, full;
    unsigned i;
    CHECK(ed_frame_build(b, a, 3, payload, 3, 1, wire, sizeof wire, &n) ==
                  ED_OK &&
              n == 64,
          "length_frame");
    for (i = 17; i < 60; ++i)
        CHECK(wire[i] == 0, "zero_padding");
    CHECK(ed_frame_parse(wire, n, 1, &f) == ED_OK && f.payload_length == 3 &&
              f.stored_payload == 46,
          "length_parse");
    CHECK(ed_frame_build(b, a, 0x800, payload, 3, 1, wire, sizeof wire, &n) ==
                  ED_OK &&
              ed_frame_parse(wire, n, 1, &f) == ED_OK && f.payload_length == 46,
          "type_frame");
    CHECK(ed_frame_build(b, a, 1520, payload, 3, 1, wire, sizeof wire, &n) ==
                  ED_INVALID &&
              ed_frame_build(b, a, 0x8100, payload, 3, 1, wire, sizeof wire,
                             &n) == ED_INVALID &&
              ed_frame_build(b, a, 2, payload, 3, 1, wire, sizeof wire, &n) ==
                  ED_INVALID,
          "ambiguous_fields");
    CHECK(ed_crc32(wire + 14, 3, &crc0) == ED_OK, "payload_crc");
    memcpy(&old, &f, sizeof f);
    wire[17] ^= 1;
    CHECK(ed_crc32(wire + 14, 3, &crc1) == ED_OK && crc0 == crc1 &&
              ed_frame_parse(wire, 64, 1, &f) == ED_IO &&
              memcmp(&f, &old, sizeof f) == 0,
          "pad_not_payload");
    CHECK(ed_frame_build(b, a, 3, payload, 3, 0, wire, sizeof wire, &n) ==
                  ED_OK &&
              n == 60,
          "hardware_tx_span");
    CHECK(ed_crc32(wire, n, &full) == ED_OK, "device_crc_model");
    for (i = 0; i < 4; ++i)
        wire[n + i] = (unsigned char)(full >> (8 * i));
    CHECK(ed_frame_parse(wire, n + 4, 1, &f) == ED_OK &&
              ed_frame_parse(wire, n, 0, &f) == ED_OK && f.stored_payload == 46,
          "capture_formats");
    CHECK(ed_frame_parse(wire, n - 4, 0, &f) == ED_INVALID, "double_subtract");
    {
        uint32_t v[] = {3, 43, 60, 64, full};
        ew_observation("frame", v, 5);
    }
    return 0;
}

static int bridge(void)
{
    struct ed_bridge s = {0}, before;
    unsigned mask = 999, i;
    unsigned char mac[6] = {2, 0, 0, 0, 1, 0};
    CHECK(ew_bridge(&s, a, b, 1, 0, 0, &mask) == ED_OK && mask == 6, "flood");
    CHECK(ew_bridge(&s, b, a, 2, 1, 0, &mask) == ED_OK && mask == 1, "learned");
    CHECK(ew_bridge(&s, a, b, 1, 2, 0, &mask) == ED_OK && mask == 2, "unicast");
    CHECK(ew_bridge(&s, a, a, 1, 3, 0, &mask) == ED_OK && mask == 0,
          "same_port");
    CHECK(ew_bridge(&s, c, b, 3, 12, 0, &mask) == ED_OK && mask == 3, "expiry");
    memset(&s, 0, sizeof s);
    for (i = 0; i < 4; ++i)
    {
        mac[5] = (unsigned char)i;
        CHECK(ew_bridge(&s, mac, group, 1, i, 0, &mask) == ED_OK, "fill");
    }
    memcpy(&before, &s, sizeof s);
    mac[5] = 4;
    mask = 999;
    CHECK(ew_bridge(&s, mac, group, 2, 4, 0, &mask) == ED_FULL &&
              memcmp(&s, &before, sizeof s) == 0 && mask == 999,
          "full_atomic");
    CHECK(ew_bridge(&s, mac, group, 2, 4, 1, &mask) == ED_OK && mask == 5 &&
              !memcmp(s.entries[0].mac, mac, 6),
          "oldest_evict");
    mac[5] = 1;
    CHECK(ew_bridge(&s, mac, a, 2, 5, 1, &mask) == ED_OK && mask == 5,
          "evicted_unknown");
    /* Equal locally administered identities move the learned port, not an IP. */
    memset(&s, 0, sizeof s);
    CHECK(ew_bridge(&s, a, b, 1, 0, 0, &mask) == ED_OK, "collision_first");
    CHECK(ew_bridge(&s, a, b, 2, 1, 0, &mask) == ED_OK, "collision_move");
    CHECK(ew_bridge(&s, c, a, 3, 2, 0, &mask) == ED_OK && mask == 2,
          "collision_misdirection");
    s.entries[3] = s.entries[0];
    memcpy(&before, &s, sizeof s);
    CHECK(ew_bridge(&s, c, a, 3, 3, 1, &mask) == ED_INVALID &&
              memcmp(&s, &before, sizeof s) == 0,
          "duplicate_state");
    CHECK((a[0] & 3) == 2 && (group[0] & 1) == 1, "mac_bits");
    {
        uint32_t v[] = {6, 1, 2, 0, 3, 4};
        ew_observation("bridge", v, 6);
    }
    return 0;
}

static int loop(void)
{
    struct ed_bridge s[2] = {0};
    unsigned wave, mask, forwarded = 0, cut = 0, i, ports[2] = {2, 3};
    /* Two parallel links between two bridges. A broadcast has no hop limit. */
    for (wave = 0; wave < 8; ++wave)
        for (i = 0; i < 2; ++i)
        {
            CHECK(ew_bridge(&s[wave % 2], a, group, ports[i], wave, 0, &mask) ==
                      ED_OK,
                  "loop_forward");
            CHECK(mask & (1u << ((ports[i] == 2 ? 3 : 2) - 1)),
                  "loop_survives");
            ++forwarded;
        }
    memset(s, 0, sizeof s);
    CHECK(ew_bridge(&s[0], a, group, 1, 0, 0, &mask) == ED_OK,
          "blocked_initial");
    mask &= ~4u;
    if (mask & 2u)
    {
        ++cut;
        CHECK(ew_bridge(&s[1], a, group, 2, 1, 0, &mask) == ED_OK,
              "blocked_peer");
        mask &= ~4u;
        CHECK(!(mask & 6u), "no_cycle");
    }
    {
        uint32_t v[] = {forwarded, cut, 4, 0};
        ew_observation("loop", v, 4);
    }
    return 0;
}

static int layers(void)
{
    uint32_t h[3] = {8, 20, 14}, sizes[3] = {99, 99, 99};
    unsigned ok = 999;
    struct ed_interface x = {1, 1, 0, 0}, y = {1, 1, 1, 0};
    struct ed_budget budget;
    CHECK(ed_wrap(12, h, 3, sizes) == ED_OK && sizes[0] == 20 &&
              sizes[1] == 40 && sizes[2] == 54,
          "encapsulation");
    h[2] = UINT32_MAX;
    CHECK(ed_wrap(12, h, 3, sizes) == ED_INVALID && sizes[2] == 54,
          "overflow_atomic");
    CHECK(ed_compatible(x, y, &ok) == ED_OK && !ok, "stream_mismatch");
    y = x;
    CHECK(ed_compatible(x, y, &ok) == ED_OK && ok, "metadata_equal");
    CHECK(ed_budget(32, 100, &budget) == ED_OK && budget.ip == 72 &&
              budget.frame == 90 && budget.slot == 110 &&
              budget.nanoseconds == 8800,
          "throughput_bound");
    {
        uint32_t v[] = {20, 40, 54, 72, 90, 110, 8800};
        ew_observation("layers", v, 7);
    }
    return 0;
}

int ew_tests(unsigned which)
{
    switch (which)
    {
        case 0:
            return stream();
        case 1:
            return vlan();
        case 2:
            return frame();
        case 3:
            return bridge();
        case 4:
            return loop();
        case 5:
            return layers();
        default:
            return 41;
    }
}
