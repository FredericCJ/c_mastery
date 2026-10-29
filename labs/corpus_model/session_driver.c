#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cm_output
{
    char bytes[8192];
    size_t used, window, pause;
    unsigned paused, calls;
};

static size_t accept(void *context, const char *bytes, size_t length)
{
    struct cm_output *o = context;
    ++o->calls;
    if (!o->paused && o->used == o->pause)
    {
        o->paused = 1;
        return 0;
    }
    if (length > o->window)
    {
        length = o->window;
    }
    if (!o->paused && length > o->pause - o->used)
    {
        length = o->pause - o->used;
    }
    if (length > sizeof o->bytes - o->used)
    {
        return 0;
    }
    memcpy(o->bytes + o->used, bytes, length);
    o->used += length;
    return length;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        return 2;
    }
    size_t chunk = (size_t)strtoul(argv[1], 0, 10),
           window = (size_t)strtoul(argv[2], 0, 10),
           pause = (size_t)strtoul(argv[3], 0, 10);
    if (!chunk || !window || pause > 8192)
    {
        return 2;
    }
    unsigned char text[8192];
    size_t length = fread(text, 1, sizeof text, stdin), position = 0;
    if (ferror(stdin) || length == sizeof text)
    {
        return 2;
    }
    struct cm_session session;
    cm_session_init(&session);
    struct cm_output out = {{0}, 0, window, pause, 0, 0};
    unsigned steps = 0;
    while (!session.closed || session.reply_length)
    {
        if (++steps > 50000)
        {
            return 3;
        }
        if (session.reply_length)
        {
            uint32_t revision = session.model.revision;
            size_t taken = 99;
            if (cm_feed(&session, (const unsigned char *)"set 0 99\n", 9,
                        &taken) != CM_BUSY ||
                taken)
            {
                return 4;
            }
            int rc = cm_drain(&session, accept, &out);
            if (rc != CM_OK && rc != CM_PENDING && rc != CM_IO)
            {
                return 5;
            }
            if (session.model.revision != revision)
            {
                return 6;
            }
        }
        else if (position < length)
        {
            size_t n = length - position, taken = 0;
            if (n > chunk)
            {
                n = chunk;
            }
            int rc = cm_feed(&session, text + position, n, &taken);
            if (rc != CM_OK && rc != CM_PENDING)
            {
                return 7;
            }
            if (!taken || taken > n)
            {
                return 8;
            }
            position += taken;
        }
        else if (cm_finish(&session) != CM_OK)
        {
            return 9;
        }
    }
    if (fwrite(out.bytes, 1, out.used, stdout) != out.used)
    {
        return 10;
    }
    fprintf(stderr, "SESSION %lu %u %lu %u\n",
            (unsigned long)session.model.revision, session.closed,
            (unsigned long)position, out.paused);
    return 0;
}
