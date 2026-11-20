#define _GNU_SOURCE
#include "supervision.h"
#include "support.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

enum { BATCH = 64, WIDTH = 8, AUDIT_ROUNDS = 4096 };
_Static_assert(SUPERVISION_FRAMES % BATCH == 0, "whole batches");
_Static_assert(BATCH * sizeof(uint32_t) <= 512, "one atomic pipe write");

static uint32_t frame_sum(const uint8_t frame[WIDTH])
{
    uint32_t sum = 0;
    for (int k = 0; k < WIDTH; ++k) sum += frame[k];
    return sum;
}

/* The finite dependent digest makes CPU service measurable. All three
 * variants use identical input, output and iteration counts. */
static uint32_t audit(uint32_t state, uint32_t value)
{
    state ^= value;
    for (int step = 0; step < AUDIT_ROUNDS; ++step)
        state = (state ^ (state >> 13)) * UINT32_C(16777619);
    return state;
}

struct connection {
    int client;
    int server;
    unsigned client_port;
    unsigned long long inode;
    char peer[INET_ADDRSTRLEN];
};

static struct connection open_loopback(void)
{
    struct connection tcp;
    struct sockaddr_in address = {.sin_family = AF_INET, .sin_port = htons(SUPERVISION_PORT),
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener < 0 || bind(listener, (struct sockaddr *)&address, sizeof address) ||
        listen(listener, 1)) fail_sys("listen on private loopback");
    tcp.client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp.client < 0 || connect(tcp.client, (struct sockaddr *)&address, sizeof address))
        fail_sys("connect to checksum service");
    struct sockaddr_in client_address;
    socklen_t length = sizeof client_address;
    tcp.server = accept(listener, (struct sockaddr *)&client_address, &length);
    if (tcp.server < 0) fail_sys("accept telemetry client");
    close(listener);
    if (!inet_ntop(AF_INET, &client_address.sin_addr, tcp.peer, sizeof tcp.peer))
        fail_sys("telemetry peer address");
    tcp.client_port = ntohs(client_address.sin_port);
    struct stat description;
    if (fstat(tcp.server, &description)) fail_sys("telemetry socket inode");
    tcp.inode = (unsigned long long)description.st_ino;
    return tcp;
}

static void barrier(char request, char expected, int64_t deadline)
{
    write_full(SUPERVISION_BARRIER_FD, &request, 1, deadline);
    char reply;
    read_full(SUPERVISION_BARRIER_FD, &reply, 1, deadline);
    if (reply != expected) fail_protocol("missing telemetry fixture snapshot");
}

static void service(int connection, int audit_out, int64_t deadline)
{
    uint32_t digest = UINT32_C(2166136261);
    for (int i = 0; i < SUPERVISION_FRAMES; i += BATCH) {
        uint8_t frames[BATCH][WIDTH];
        uint32_t replies[BATCH];
        read_full(connection, frames, sizeof frames, deadline);
        for (int j = 0; j < BATCH; ++j) {
            uint32_t value = frame_sum(frames[j]);
            digest = audit(digest, value);
            replies[j] = htonl(value);
        }
        write_full(connection, replies, sizeof replies, deadline);
    }
    write_full(audit_out, &digest, sizeof digest, deadline);
    close(audit_out);
    close(connection);
    _exit(0);
}

struct delivered {
    int frames;
    unsigned long long checksum;
};

/* A positive grant releases exactly that many queued batches. The final
 * zero grant closes the finite stream and requests the sink's total. */
static void sink(int input, int grants, int acknowledgments, int64_t deadline)
{
    struct delivered total = {0, 0};
    for (;;) {
        unsigned batches;
        read_full(grants, &batches, sizeof batches, deadline);
        if (!batches) break;
        for (unsigned b = 0; b < batches; ++b) {
            uint32_t replies[BATCH];
            read_full(input, replies, sizeof replies, deadline);
            for (int j = 0; j < BATCH; ++j) {
                uint32_t value = ntohl(replies[j]);
                uint32_t expected = 0;
                for (int k = 0; k < WIDTH; ++k) expected += (total.frames + k) & 255;
                if (value != expected) fail_protocol("wrong downstream reply or order");
                total.checksum += value;
                ++total.frames;
            }
        }
        write_full(acknowledgments, "D", 1, deadline);
    }
    write_full(acknowledgments, &total, sizeof total, deadline);
    _exit(0);
}

static void drain(unsigned queued, int grants, int acknowledgments, int64_t deadline)
{
    write_full(grants, &queued, sizeof queued, deadline);
    char done;
    read_full(acknowledgments, &done, 1, deadline);
    if (done != 'D') fail_protocol("missing sink acknowledgment");
}

static ssize_t try_batch(int out, const uint32_t replies[BATCH])
{
    ssize_t count;
    do {
        count = write(out, replies, BATCH * sizeof replies[0]);
    } while (count < 0 && errno == EINTR);
    return count;
}

int supervise_telemetry(const char *variant)
{
    int stalled = !strcmp(variant, "stalled");
    if (!stalled && strcmp(variant, "baseline") && strcmp(variant, "quota")) return 2;
    char leaf[256], policy[64], quota[32];
    unsigned period;
    leaf_membership(leaf, sizeof leaf);
    leaf_field(leaf, "cpu.max", policy, sizeof policy);
    if (sscanf(policy, "%31s %u", quota, &period) != 2) fail_protocol("invalid cpu.max");
    int64_t deadline = now_ms() + 15000;
    struct connection tcp = open_loopback();
    int audit_pipe[2];
    if (pipe(audit_pipe)) fail_sys("audit pipe");
    unsigned long long usage_before = leaf_counter(leaf, "cpu.stat", "usage_usec");
    unsigned long long throttle_before = leaf_counter(leaf, "cpu.stat", "nr_throttled");
    barrier('R', 'G', deadline);
    int64_t start = now_ms();
    pid_t server = fork();
    if (server < 0) fail_sys("service fork");
    if (!server) {
        close(SUPERVISION_BARRIER_FD);
        close(tcp.client);
        close(audit_pipe[0]);
        service(tcp.server, audit_pipe[1], deadline);
    }
    close(tcp.server);
    close(audit_pipe[1]);

    int stream[2], grants[2], acknowledgments[2];
    if (pipe(stream) || pipe(grants) || pipe(acknowledgments)) fail_sys("sink channels");
    if (fcntl(stream[1], F_SETPIPE_SZ, 4096) < 0) fail_sys("F_SETPIPE_SZ");
    int capacity = fcntl(stream[1], F_GETPIPE_SZ);
    int flags = fcntl(stream[1], F_GETFL);
    if (capacity < 0 || flags < 0 || fcntl(stream[1], F_SETFL, flags | O_NONBLOCK))
        fail_sys("pipe flags");
    pid_t reader = fork();
    if (reader < 0) fail_sys("sink fork");
    if (!reader) {
        close(SUPERVISION_BARRIER_FD);
        close(tcp.client);
        close(audit_pipe[0]);
        close(stream[1]);
        close(grants[1]);
        close(acknowledgments[0]);
        sink(stream[0], grants[0], acknowledgments[1], deadline);
    }
    close(stream[0]);
    close(grants[0]);
    close(acknowledgments[1]);

    int eagain = 0, resumed = 0, pause_reader = stalled;
    unsigned queued = 0, tcp_reply_bytes = 0;
    for (int i = 0; i < SUPERVISION_FRAMES; i += BATCH) {
        uint8_t request[BATCH][WIDTH];
        uint32_t replies[BATCH];
        for (int j = 0; j < BATCH; ++j)
            for (int k = 0; k < WIDTH; ++k)
                request[j][k] = (uint8_t)(i + j + k);
        write_full(tcp.client, request, sizeof request, deadline);
        read_full(tcp.client, replies, sizeof replies, deadline);
        tcp_reply_bytes += sizeof replies;
        /* With a single nonblocking writer, a batch <= PIPE_BUF is atomic. */
        ssize_t n = try_batch(stream[1], replies);
        if (n < 0 && errno == EAGAIN) {
            ++eagain;
            if (!queued) fail_protocol("refusal without outstanding replies");
            drain(queued, grants[1], acknowledgments[0], deadline);
            queued = 0;
            pause_reader = 0;
            n = try_batch(stream[1], replies);
            if (n == (ssize_t)sizeof replies) ++resumed;
        }
        if (n != (ssize_t)sizeof replies) fail_protocol("batch was not delivered");
        ++queued;
        if (!pause_reader) {
            drain(queued, grants[1], acknowledgments[0], deadline);
            queued = 0;
        }
    }
    if (queued) drain(queued, grants[1], acknowledgments[0], deadline);
    unsigned finish = 0;
    write_full(grants[1], &finish, sizeof finish, deadline);
    close(grants[1]);
    close(stream[1]);
    close(tcp.client);
    struct delivered total;
    read_full(acknowledgments[0], &total, sizeof total, deadline);
    close(acknowledgments[0]);
    uint32_t digest;
    read_full(audit_pipe[0], &digest, sizeof digest, deadline);
    close(audit_pipe[0]);
    pid_t children[] = {reader, server};
    for (size_t i = 0; i < sizeof children / sizeof children[0]; ++i) {
        int status = collect(children[i], deadline);
        if (!WIFEXITED(status) || WEXITSTATUS(status)) fail_protocol("telemetry worker failed");
    }
    barrier('D', 'F', deadline);
    close(SUPERVISION_BARRIER_FD);
    unsigned long long usage_after = leaf_counter(leaf, "cpu.stat", "usage_usec");
    unsigned long long throttle_after = leaf_counter(leaf, "cpu.stat", "nr_throttled");
    printf("LS telemetry variant=%s cgroup=%s quota=%s period=%u frames=%d bytes=%d "
           "checksum=%llu digest=%" PRIu32 " usage_before=%llu usage_after=%llu "
           "throttle_before=%llu throttle_after=%llu eagain=%d resumed=%d "
           "peer=%s port=%d client_port=%u server_inode=%llu tcp_reply_bytes=%u "
           "pipe_size=%d elapsed_ms=%" PRId64 "\n", variant, leaf, quota, period,
           total.frames, total.frames * WIDTH, total.checksum, digest, usage_before,
           usage_after, throttle_before, throttle_after, eagain, resumed,
           tcp.peer, SUPERVISION_PORT, tcp.client_port, tcp.inode, tcp_reply_bytes, capacity,
           now_ms() - start);
    return 0;
}
