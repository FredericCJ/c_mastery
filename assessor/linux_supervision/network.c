#define _POSIX_C_SOURCE 200809L
#include "supervision.h"
#include "support.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

static int tcp_socket(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) fail_sys("TCP socket");
    return fd;
}

int supervise_network(const char *variant)
{
    static const uint8_t request[] = {0, 1, 2, 3, 128, 254, 255, 0};
    if (strcmp(variant, "A") && strcmp(variant, "B")) return 2;
    int64_t deadline = now_ms() + 6000;
    struct sockaddr_in address = {.sin_family = AF_INET, .sin_port = htons(SUPERVISION_PORT)};
    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) != 1)
        fail_protocol("loopback address");
    int listener = tcp_socket();
    if (bind(listener, (struct sockaddr *)&address, sizeof address)) fail_sys("bind");
    if (listen(listener, 1)) fail_sys("listen");

    write_full(SUPERVISION_BARRIER_FD, "R", 1, deadline);
    char go;
    read_full(SUPERVISION_BARRIER_FD, &go, 1, deadline);
    if (go != 'G') fail_protocol("invalid network barrier");
    close(SUPERVISION_BARRIER_FD);

    int duplicate = tcp_socket();
    int result = bind(duplicate, (struct sockaddr *)&address, sizeof address);
    int duplicate_errno = result < 0 ? errno : 0;
    close(duplicate);

    pid_t server = fork();
    if (server < 0) fail_sys("server fork");
    if (server == 0) {
        int connection = accept(listener, NULL, NULL);
        if (connection < 0) fail_sys("accept");
        close(listener);
        /* A one-octet endpoint greeting precedes the unchanged 8 -> 4 codec. */
        write_full(connection, variant, 1, deadline);
        uint8_t bytes[sizeof request];
        read_full(connection, bytes, sizeof bytes, deadline);
        uint32_t sum = 0;
        for (size_t i = 0; i < sizeof bytes; ++i) sum += bytes[i];
        uint32_t wire = htonl(sum);
        write_full(connection, &wire, sizeof wire, deadline);
        close(connection);
        _exit(0);
    }

    /* Keep our listener through connect so that its namespace-local identity
     * stays live independently of when the server child is scheduled. */
    int client = tcp_socket();
    if (connect(client, (struct sockaddr *)&address, sizeof address)) fail_sys("connect");
    close(listener);
    unsigned char marker;
    read_full(client, &marker, 1, deadline);
    write_full(client, request, sizeof request, deadline);
    uint32_t reply;
    read_full(client, &reply, sizeof reply, deadline);
    close(client);
    int status = collect(server, deadline);
    if (!WIFEXITED(status) || WEXITSTATUS(status)) fail_protocol("server failed");
    printf("LS network variant=%s net_ns=%llu port=%d marker=%u reply=%u "
           "duplicate_errno=%d\n", variant, namespace_inode("net"), SUPERVISION_PORT,
           (unsigned)marker, (unsigned)ntohl(reply), duplicate_errno);
    return 0;
}
