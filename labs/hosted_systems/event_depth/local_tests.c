#include "local.h"
#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#define CHECK(c)                                                          \
    do                                                                    \
    {                                                                     \
        if (!(c))                                                         \
        {                                                                 \
            fprintf(stderr, "LOCAL-CHECK:%s:%d errno=%d\n", #c, __LINE__, \
                    errno);                                               \
            return 1;                                                     \
        }                                                                 \
    } while (0)

int main(void)
{
    struct el_address v4, v6, unix_address;
    CHECK(el_loopback(AF_INET, 0x1234, &v4) == 0 &&
          v4.length == sizeof(struct sockaddr_in));
    struct sockaddr_in a;
    memcpy(&a, &v4.storage, sizeof a);
    CHECK(ntohs(a.sin_port) == 0x1234 &&
          ntohl(a.sin_addr.s_addr) == INADDR_LOOPBACK);
    unsigned char port[2];
    memcpy(port, &a.sin_port, 2);
    CHECK(port[0] == 0x12 && port[1] == 0x34);
    CHECK(el_loopback(AF_INET6, 0x1234, &v6) == 0 &&
          v6.length == sizeof(struct sockaddr_in6));
    struct sockaddr_in6 b;
    memcpy(&b, &v6.storage, sizeof b);
    CHECK(ntohs(b.sin6_port) == 0x1234 && IN6_IS_ADDR_LOOPBACK(&b.sin6_addr));
    CHECK(el_pathname("/tmp/original-cmastery-example/socket", &unix_address) ==
          0);
    CHECK(unix_address.length ==
          offsetof(struct sockaddr_un, sun_path) +
              strlen("/tmp/original-cmastery-example/socket") + 1);
    unsigned char before[sizeof v4];
    memcpy(before, &v4, sizeof v4);
    CHECK(el_loopback(AF_UNSPEC, 7, &v4) == -1 && errno == EAFNOSUPPORT &&
          memcmp(before, &v4, sizeof v4) == 0);
    char too_long[256];
    memset(too_long, 'x', sizeof too_long);
    too_long[0] = '/';
    too_long[255] = 0;
    CHECK(el_pathname(too_long, &v4) == -1 && errno == ENAMETOOLONG);
    puts(
        "LOCAL-PASS:families=3;port_wire=1234;unsupported_preserves=1;pathname_"
        "length=offsetof_plus_name_plus_nul;");
    return 0;
}
