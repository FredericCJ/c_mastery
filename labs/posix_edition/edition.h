#ifndef CM_POSIX_EDITION_H
#define CM_POSIX_EDITION_H
/* Contract model: these values are not errno numbers or OS feature probes. */
enum PEVersion { PE_2017, PE_LINUX, PE_2024 };
enum PECall { PE_CLOSE, PE_PC_ZERO, PE_PC_RESTART, PE_PC_INVALID };
enum PEOutcome { PE_OK, PE_INTR, PE_PROGRESS, PE_IO, PE_BADF, PE_INVAL, PE_AGAIN };
enum PEAction { PE_RETIRED, PE_RETRY_CLOSE_ONLY, PE_UNRESOLVED, PE_INVALID_HANDLE };
typedef struct { enum PEAction action; unsigned diagnostic; } PEDecision;
/* 0 publishes a decision; 1 rejects an outcome outside this declared model.
 * restart is exactly 0 or 1: whether POSIX_CLOSE_RESTART is nonzero.
 * Unsupported input and null output leave all caller state unchanged. */
int pe_close(enum PEVersion, enum PECall, unsigned restart,
             enum PEOutcome, PEDecision *);
enum PERead { PE_ACQUIRE, PE_BLOCK, PE_IMPL_CHOICE, PE_MAY_DEADLOCK, PE_UNDEFINED };
/* Selected no-TPS or homogeneous FIFO/RR/SPORADIC population only.
 * priority_waiter means at least one waiting writer of equal/higher priority.
 * Not a lock, scheduler, fairness guarantee, or test of arbitrary mixed policy. */
int pe_read(unsigned initialized, unsigned self_writer, unsigned other_writer,
            unsigned self_reader, unsigned writer_waiting, unsigned tps,
            unsigned priority_waiter, enum PERead *);
#endif
