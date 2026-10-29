#ifndef TRANSCEIVERS_H
#define TRANSCEIVERS_H
#include <stddef.h>
#include <stdint.h>

enum { TR_OK, TR_INVALID, TR_INCOMPLETE, TR_MISMATCH, TR_BUSY, TR_STALE,
       TR_UNIMPLEMENTED };
enum { TR_UNKNOWN = -1, TR_LOW = 0, TR_HIGH = 1, TR_HIGH_Z = 2 };
enum { TR_RS232_FIXTURE, TR_MAX3232_33, TR_RS422_FIXTURE, TR_AM26LV32E };
struct tr_sample {
    unsigned device, enabled, open, swapped;
    int32_t a_mv, b_mv;
};
/* Invalid input preserves *decision. A disconnected fixture has no bit;
   the selected powered AM26LV32E's open-input guarantee is a separate case. */
int tr_sample(const struct tr_sample *sample, int *decision);

enum { TR_DTE, TR_DCE };
struct tr_connector { unsigned role, td, rd, reference; };
struct tr_wire { unsigned left, right; };
/* Our original three-contact teaching adapter, not a generic DB9 mapping. */
int tr_connections(const struct tr_connector *left, const struct tr_connector *right,
                   const struct tr_wire *wires, size_t count);
int tr_topology(unsigned active_drivers, unsigned receivers,
                unsigned terminated_at_far_end, unsigned reference);

struct tr232_circuit {
    uint32_t supply_mv, baud, resistance_ohm, capacitance_pf;
    uint32_t pump_nf[4], bypass_nf;
    int32_t temperature_c;
};
/* Admission for this laboratory's conservative selected MAX3232I circuit. */
int tr_232_circuit(const struct tr232_circuit *circuit);

struct tr_point {
    uint64_t time_ns;
    int32_t a_mv, b_mv;
    unsigned expected, returned, columns;
};
enum { TR_HAVE_LOGIC = 1, TR_HAVE_LINE = 2, TR_HAVE_RETURN = 4,
       TR_HAVE_PAIR = 8 };
struct tr_result { size_t matched, unknown, mismatched, first_bad; };
/* Point samples are aligned settled bit centers supplied by the capture
   adapter. This is not a waveform decoder or proof of setup/hold margins. */
int tr_capture(unsigned device, const struct tr_point *points, size_t count,
               unsigned swapped, struct tr_result *result);
struct tr_edge { unsigned transitions, opposite, center, outside; };
/* Inspect one declared transition window, no causal claim from samples alone. */
int tr_edge(const struct tr_point *points, size_t count, unsigned expected,
            unsigned swapped, struct tr_edge *result);

struct tr_bus {
    uint32_t serial;
    uint64_t last, ready_at;
    unsigned owner, phase;
};
/* Separate three-peer token policy for a half-duplex multipoint variant.
   No electrical RS-485 device is implemented by this scheduler. */
int tr_grant(struct tr_bus *bus, unsigned peer, uint32_t serial, uint64_t now);
int tr_release(struct tr_bus *bus, unsigned peer, uint32_t serial, uint64_t now,
               unsigned transmitter_complete, uint64_t disable_ns,
               uint64_t guard_ns);
int tr_ready(struct tr_bus *bus, uint64_t now);
int tr_test(unsigned selector);
#endif
