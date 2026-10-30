/*
 * Writes a short string to the file named by argv[1], rewinds, reads it back
 * and compares. Exit status: 0 when it round-trips, 1 when any step fails or
 * the text differs, 2 for a wrong argument count, 3 when the file cannot be
 * opened. The file is overwritten.
 */
#include <stdio.h>
#include <string.h>

#define TEXT "synthetic"
#define READ_BYTES 16u

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    FILE *stream = fopen(argv[1], "w+");
    if (!stream)
    {
        return 3;
    }
    int failed =
        fputs(TEXT, stream) < 0 || fflush(stream) || fseek(stream, 0, SEEK_SET);
    char read_back[READ_BYTES];
    if (!failed)
    {
        failed = !fgets(read_back, sizeof read_back, stream) ||
                 strcmp(read_back, TEXT);
    }
    if (fclose(stream))
    {
        failed = 1;
    }
    return failed;
}
