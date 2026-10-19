#ifndef TRANSCEIVERS_H
#define TRANSCEIVERS_H
#include <stddef.h>
#include <stdint.h>

enum { TR_OK, TR_INVALID, TR_INCOMPLETE, TR_MISMATCH, TR_BUSY, TR_STALE,
       TR_UNIMPLEMENTED };
enum { TR_UNKNOWN = -1, TR_LOW = 0, TR_HIGH = 1, TR_HIGH_Z = 2 };
enum { TR_RS232_FIXTURE, TR_MAX3232_33, TR_RS422_FIXTURE, TR_AM26LV32E };
typedef struct {
    unsigned device, enabled, open, swapped;
    int32_t a_mv, b_mv;
} TrSample;
/* Invalid input preserves *decision. A disconnected fixture has no bit;
   the selected powered AM26LV32E's open-input guarantee is a separate case. */
int tr_sample(const TrSample *sample, int *decision);

enum { TR_DTE, TR_DCE };
typedef struct { unsigned role, td, rd, reference; } TrConnector;
typedef struct { unsigned left, right; } TrWire;
/* Our original three-contact teaching adapter, not a generic DB9 mapping. */
int tr_connections(const TrConnector *left, const TrConnector *right,
                   const TrWire *wires, size_t count);
int tr_topology(unsigned active_drivers, unsigned receivers,
                unsigned terminated_at_far_end, unsigned reference);

typedef struct {
    uint32_t supply_mv, baud, resistance_ohm, capacitance_pf;
    uint32_t pump_nf[4], bypass_nf;
    int32_t temperature_c;
} Tr232Circuit;
/* Admission for this laboratory's conservative selected MAX3232I circuit. */
int tr_232_circuit(const Tr232Circuit *circuit);

typedef struct {
    uint64_t time_ns;
    int32_t a_mv, b_mv;
    unsigned expected, returned, columns;
} TrPoint;
enum { TR_HAVE_LOGIC = 1, TR_HAVE_LINE = 2, TR_HAVE_RETURN = 4,
       TR_HAVE_PAIR = 8 };
typedef struct { size_t matched, unknown, mismatched, first_bad; } TrResult;
/* Point samples are aligned settled bit centers supplied by the capture
   adapter. This is not a waveform decoder or proof of setup/hold margins. */
int tr_capture(unsigned device, const TrPoint *points, size_t count,
               unsigned swapped, TrResult *result);
typedef struct { unsigned transitions, opposite, center, outside; } TrEdge;
/* Inspect one declared transition window, no causal claim from samples alone. */
int tr_edge(const TrPoint *points, size_t count, unsigned expected,
            unsigned swapped, TrEdge *result);

typedef struct {
    uint32_t serial;
    uint64_t last, ready_at;
    unsigned owner, phase;
} TrBus;
/* Separate three-peer token policy for a half-duplex multipoint variant.
   No electrical RS-485 device is implemented by this scheduler. */
int tr_grant(TrBus *bus, unsigned peer, uint32_t serial, uint64_t now);
int tr_release(TrBus *bus, unsigned peer, uint32_t serial, uint64_t now,
               unsigned transmitter_complete, uint64_t disable_ns,
               uint64_t guard_ns);
int tr_ready(TrBus *bus, uint64_t now);
int tr_test(unsigned selector);
#endif
