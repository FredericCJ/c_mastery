typedef unsigned long long po_u;

struct po_result
{
    po_u sum, first_xor_last, last;
};

__attribute__((noinline)) struct po_result
po_aggregate(po_u a, po_u b, po_u c, po_u d, po_u e, po_u f, po_u g, po_u h)
{
    struct po_result r = {a + b + c + d + e + f + g + h, a ^ h, h};
    return r;
}
