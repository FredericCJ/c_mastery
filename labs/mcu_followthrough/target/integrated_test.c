#include "port.h"

/* Fixture admission/ownership oracle. All shared accesses are in the same
 * saved-PRIMASK domain, including UART and SysTick producers. */
struct mf_sample
{
    uint32_t id, value, lease;
};

struct mf_samples
{
    struct mf_sample row[4];
    unsigned head, count;
};
static struct mf_samples input, output;
static uint32_t pool[4], generation[4];
static unsigned owner[4], state[128], attempted, accepted, rejected, processed,
    delivered, output_drop;
static unsigned logs[2], log_count, log_drop, recovery_done, replacement_done,
    max_isr_batch;
static uint32_t received_checksum, expected_checksum, computed, consumed,
    dropped_sum, values[128];
static uint32_t stacks[5][768] __attribute__((aligned(8)));
static mf_handle_id handles[5], old_fault;
static volatile uint32_t hog_iterations;

static void log_event(unsigned value)
{
    if (log_count < 2)
        logs[log_count++] = value;
    else
        log_drop++;
}

static void publish(unsigned value)
{
    unsigned id = attempted++;
    mf_require(id < 128, 230);
    values[id] = value;
    if (input.count == 4)
    {
        state[id] = 2;
        rejected++;
        log_event(1);
        return;
    }
    unsigned slot = 0;
    while (slot < 4 && owner[slot])
        slot++;
    if (slot == 4)
    {
        state[id] = 2;
        rejected++;
        log_event(1);
        return;
    }
    pool[slot] = value;
    owner[slot] = 1;
    struct mf_sample s = {id, value, (generation[slot] << 3) | (slot + 1u)};
#ifdef MF_BORROWED
    s.value =
        0; /* Stable pool lease replaces copied data; producer cannot reuse. */
#endif
    input.row[(input.head + input.count) % 4] = s;
    input.count++;
    state[id] = 1;
    accepted++;
    received_checksum += value;
    log_event(2);
}

void mf_tick_hook(void)
{
#ifndef MF_UART
    if (mf_ticks <= 30 && (mf_ticks % 2u) == 0)
    {
        unsigned n = mf_ticks == 6 ? 12 : 1;
        if (n > max_isr_batch)
            max_isr_batch = n;
        for (unsigned i = 0; i < n; i++)
            publish(mf_ticks * 10u + i);
    }
#endif
}

void UART0_Handler(void)
{
    unsigned p = mf_lock(), n = 0;
    MF_REG(0x4000400c) = 2;
    while ((MF_REG(0x40004004) & 2u) && n < 4)
    {
        publish(MF_REG(0x40004000) & 255u);
        n++;
    }
    if (n > max_isr_batch)
        max_isr_batch = n;
    mf_unlock(p);
}

static void processor(void)
{
    for (;;)
    {
        unsigned p = mf_lock();
        if (!input.count)
        {
            mf_unlock(p);
            continue;
        }
        struct mf_sample s = input.row[input.head];
        input.head = (input.head + 1u) % 4;
        input.count--;
        unsigned slot = (s.lease & 7u) - 1u;
        mf_require(slot < 4 && owner[slot] && generation[slot] == s.lease >> 3,
                   232);
#ifdef MF_BORROWED
        unsigned value = pool[slot];
#else
        unsigned value = s.value;
        mf_require(value == pool[slot], 233);
#endif
        mf_unlock(p);
        volatile unsigned work = value;
        for (unsigned i = 0; i < 8; i++)
            work = work * 33u + 17u;
        p = mf_lock();
        mf_require(owner[slot] && pool[slot] == value, 234);
        owner[slot] = 0;
        generation[slot]++;
        processed++;
        computed += work;
        state[s.id] = 3;
        if (output.count == 2)
        {
            output_drop++;
            dropped_sum += work;
            state[s.id] = 4;
            log_event(3);
        }
        else
        {
            s.value = work;
            output.row[(output.head + output.count) % 4] = s;
            output.count++;
        }
        mf_unlock(p);
    }
}

static void sink(void)
{
    for (;;)
    {
        if (mf_ticks < 18)
            continue;
        unsigned p = mf_lock();
        if (output.count)
        {
            struct mf_sample s = output.row[output.head];
            output.head = (output.head + 1u) % 4;
            output.count--;
            consumed += s.value;
            state[s.id] = 5;
            delivered++;
        }
        /* Bounded diagnostic consumer starts after the recovery pressure window. */
        if (log_count)
        {
            mf_require(logs[0] > 0, 235);
            logs[0] = logs[1];
            log_count--;
        }
        mf_unlock(p);
    }
}

static void failed_worker(void)
{
    while (mf_ticks < 7)
        __asm__ volatile("nop");
    __asm__ volatile("udf #91");
    mf_require(0, 236);
}

static void replacement(void)
{
    replacement_done = 1;
}

static void supervisor(void)
{
    for (;;)
    {
        unsigned p = mf_lock();
        if (mf_fault_count && !recovery_done)
        {
            int result;
            old_fault = handles[2];
            mf_require(mf_join(&mf_machine, handles[3], old_fault,
                               cmx_forever(), &result) == CMX_OK &&
                           result == -1,
                       237);
            mf_require(mf_reap(&mf_machine, old_fault) == CMX_OK &&
                           mf_lookup(&mf_machine, old_fault) == -1,
                       238);
            handles[2] = mf_port_add(replacement, stacks[2], 768);
            mf_require(handles[2] != old_fault, 239);
            recovery_done = 1;
            log_event(4);
        }
#ifdef MF_UART
        unsigned done =
            attempted == 64 && mf_ticks >= 40 && !input.count && !output.count;
#else
        unsigned done = mf_ticks >= 40 && !input.count && !output.count;
#endif
        if (done)
        {
            MF_REG(0xe000e010) = 0;
            MF_REG(0x40004008) = 0;
            mf_require(recovery_done && replacement_done &&
                           mf_fault_count == 1 && hog_iterations,
                       240);
            mf_require(attempted == accepted + rejected &&
                           accepted == processed &&
                           processed == delivered + output_drop,
                       241);
            mf_require(rejected && output_drop && log_drop, 242);
            for (unsigned i = 0; i < attempted; i++)
            {
                mf_require(state[i] == 2 || state[i] == 4 || state[i] == 5,
                           243);
                if (state[i] != 2)
                    expected_checksum += values[i];
            }
            mf_require(received_checksum == expected_checksum &&
                           computed == consumed + dropped_sum,
                       246);
            for (unsigned i = 0; i < 4; i++)
                mf_require(!owner[i], 244);
            mf_text(
                "MF attempted/accepted/rejected/processed/delivered/output-drop/log-drop/max-ISR-batch\n");
            mf_number(attempted);
            mf_number(accepted);
            mf_number(rejected);
            mf_number(processed);
            mf_number(delivered);
            mf_number(output_drop);
            mf_number(log_drop);
            mf_number(max_isr_batch);
            mf_text(
                "MF sums admitted/input-model/computed/consumed; all final sample states\n");
            mf_number(received_checksum);
            mf_number(expected_checksum);
            mf_number(computed);
            mf_number(consumed);
            for (unsigned i = 0; i < attempted; i++)
            {
                mf_number(i);
                mf_number(state[i]);
            }
            mf_text("MF recovery old/new generation handles\n");
            mf_number((uint32_t)old_fault);
            mf_number((uint32_t)handles[2]);
            mf_text(
                "MF RAM input/output/pool+leases/stacks/kernel+wrapper/diagnostics\n");
            mf_number(sizeof input);
            mf_number(sizeof output);
            mf_number(sizeof pool + sizeof generation + sizeof owner);
            mf_number(sizeof stacks);
            mf_number(sizeof mf_machine);
            mf_number(sizeof logs);
            mf_text("PASS MF integrated\n");
            mf_finish(0);
        }
        mf_require(mf_ticks < 400, 245);
        mf_unlock(p);
        mf_pend();
    }
}

static void hog(void)
{
    for (;;)
        hog_iterations++;
}

void mf_main(void)
{
    mf_port_init();
    void (*entry[5])(void) = {processor, sink, failed_worker, supervisor, hog};
    for (unsigned i = 0; i < 5; i++)
        handles[i] = mf_port_add(entry[i], stacks[i], 768);
#ifdef MF_UART
    MF_REG(0x40004010) = 217;
    MF_REG(0x40004008) = 10;
    MF_REG(0xe000e100) = 1;
    mf_text("MF UART ready\n");
#endif
    mf_port_start();
}
