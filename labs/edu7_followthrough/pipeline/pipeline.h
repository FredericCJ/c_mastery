#ifndef EDU7_PIPELINE_SUCCESSOR_H
#define EDU7_PIPELINE_SUCCESSOR_H
#include "physical.h"
#include "session.h"

enum
{
    EW_OK,
    EW_INVALID,
    EW_BUSY,
    EW_STUB = 99
};

struct ew_port
{
    unsigned master, mode, queued, data_seen;
    struct edu_frame queue;
    struct ef_framer parser;
    struct ep_master m;
    struct ep_slave s;
    uint32_t frames, bytes, faults, drops;
};

struct ew_node
{
    unsigned node, profile, ports, event_ready, paused;
    uint32_t last, network, forwarded, route_drops, transport_drops,
        upper_drops, ack_drops;
    struct ew_port port[2];
    struct ef_route route;
    unsigned route_protocol;
    struct es_peer endpoint;
    struct es_event event;
    uint8_t incoming[40], outgoing[40];
    size_t incoming_length, outgoing_length;
};

int ew_init(struct ew_node *, unsigned, unsigned, uint16_t,
            const struct r5_io *, const struct pu_clock *, uint32_t);
int ew_queue(struct ew_node *, unsigned, const struct edu_frame *);
int ew_tick(struct ew_node *, uint32_t);
int ew_take(struct ew_node *, struct es_event *);
int ew_pause(struct ew_node *);
int ew_drained(const struct ew_node *);
int ew_rebind(struct ew_node *, uint16_t);
int ew_run(struct ew_node *);
int ew_f439_open(struct ew_node *, unsigned, unsigned, uint16_t,
                 const struct pu_clock *, uint32_t);
int ew_physical_verdict(unsigned software_ok, unsigned measured_mask);
int ew_tests(unsigned);
#endif
