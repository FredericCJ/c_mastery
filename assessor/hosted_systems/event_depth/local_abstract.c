#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#define CHECK(c)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(c))                                                        \
        {                                                                \
            fprintf(stderr, "LOCAL-ABSTRACT-CHECK:%s:%d errno=%d\n", #c, \
                    __LINE__, errno);                                    \
            return 1;                                                    \
        }                                                                \
    } while (0)

int main(void)
{
    alarm(5);
    struct sockaddr_un a = {0}, b = {0};
    a.sun_family = AF_UNIX;
    int n = snprintf(a.sun_path + 1, sizeof a.sun_path - 3,
                     "cmastery-original-%ld", (long)getpid());
    CHECK(n > 0 && (size_t)n < sizeof a.sun_path - 3);
    /* Leading abstract marker, name, embedded NUL, and one significant suffix. */
    a.sun_path[n + 2] = 'X';
    b = a;
    b.sun_path[n + 2] = 'Y';
    socklen_t len =
        (socklen_t)(offsetof(struct sockaddr_un, sun_path) + (size_t)n + 3);
    int x = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0),
        y = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0),
        z = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    CHECK(x >= 0 && y >= 0 && z >= 0);
    CHECK(!strcmp(a.sun_path + 1, b.sun_path + 1) &&
          memcmp(a.sun_path, b.sun_path, (size_t)n + 3) != 0);
    CHECK(bind(x, (struct sockaddr *)&a, len) == 0 &&
          bind(y, (struct sockaddr *)&b, len) == 0);
    CHECK(bind(z, (struct sockaddr *)&a, len) == -1 && errno == EADDRINUSE);
    CHECK(sendto(z, "X", 1, 0, (struct sockaddr *)&a, len) == 1 &&
          sendto(z, "Y", 1, 0, (struct sockaddr *)&b, len) == 1);
    char value;
    CHECK(recv(x, &value, 1, 0) == 1 && value == 'X');
    CHECK(recv(y, &value, 1, 0) == 1 && value == 'Y');
    CHECK(close(x) == 0 && bind(z, (struct sockaddr *)&a, len) == 0);
    CHECK(close(z) == 0 && close(y) == 0);
    puts(
        "LOCAL-ABSTRACT:embedded_nul_names_distinct=1;exact_length=1;duplicate_"
        "rejected=1;last_close_releases_name=1;filesystem_permissions_not_"
        "claimed=1;");
    return 0;
}
