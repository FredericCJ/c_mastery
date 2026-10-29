#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        return 2;
    }
    for (int i = 1; i < 4; i++)
    {
        FILE *f = fopen(argv[i], "r");
        if (!f)
        {
            return 2;
        }
        char b[1024];
        while (fgets(b, sizeof b, f))
        {
            if ((i == 1 && strspn(b, " \r\n\t") != strlen(b)) ||
                strstr(b, "NEEDED") || strstr(b, "INTERP"))
            {
                fclose(f);
                return 1;
            }
            if (!strchr(b, '\n') && !feof(f))
            {
                fclose(f);
                return 2;
            }
        }
        if (ferror(f))
        {
            fclose(f);
            return 2;
        }
        fclose(f);
    }
    return 0;
}
