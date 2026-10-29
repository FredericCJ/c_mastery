#include "cvm.h"
#include <stdio.h>
#include <string.h>

static int read_file(const char *name, unsigned char *bytes, size_t cap,
                     size_t *n)
{
    FILE *f = fopen(name, "rb");
    if (!f)
        return 0;
    *n = fread(bytes, 1, cap, f);
    int extra = fgetc(f), bad = ferror(f), closed = fclose(f);
    return extra == EOF && !bad && !closed;
}
#ifdef VP_COMPILER
int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    unsigned char source[4096], image[1100];
    size_t count, n = 0;
    if (!read_file(argv[1], source, sizeof source, &count))
        return 3;
    struct cvm_program p;
    enum cvm_status status = cvm_compile((const char *)source, count, &p);
    if (status)
    {
        printf("VP_COMPILE_REFUSE %u\n", (unsigned)status);
        return 1;
    }
    enum cvm_status verified = cvm_verify(&p);
    if (verified || cvm_save(&p, image, sizeof image, &n))
        return 4;
    FILE *f = fopen(argv[2], "wbx");
    if (!f)
        return 5;
    size_t written = fwrite(image, 1, n, f);
    int closed = fclose(f);
    if (written != n || closed)
        return 6;
    printf("VP_COMPILE status=0 verify=0 bytes=%zu\n", n);
    return 0;
}
#else
static unsigned calls;
static int adapter;

static int host(void *ctx, uint8_t id, int32_t input, int32_t *out)
{
    (void)ctx;
    ++calls;
    if (id != 1 || input > 2147483645)
        return -1;
    *out = input + adapter;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3 || ((adapter = argv[2][0] - '0') != 1 && adapter != 2) ||
        argv[2][1])
        return 2;
    unsigned char bytes[1100];
    size_t n;
    if (!read_file(argv[1], bytes, sizeof bytes, &n))
        return 3;
    struct cvm_program p;
    enum cvm_status loaded = cvm_load(bytes, n, &p);
    if (loaded)
    {
        printf("VP_LOAD_REFUSE %u\n", (unsigned)loaded);
        return 1;
    }
    struct cvm_cvm vm;
    cvm_init(&vm, 3, 0, 1);
    vm.host = host;
    struct cvm_value result = cvm_int(91);
    enum cvm_status status = cvm_run(&vm, &p, &result);
    printf("{\"status\":%u,\"value\":%d,\"fuel_left\":%u,\"host_left\":%u,"
           "\"calls\":%u,\"adapter\":%d}\n",
           (unsigned)status, result.integer, vm.fuel, vm.host_budget, calls,
           adapter);
    return status != CVM_OK;
}
#endif
