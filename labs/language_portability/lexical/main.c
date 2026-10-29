_Static_assert(sizeof(int) == 4 && sizeof(long) == 8, "selected host model");
unsigned lp_a(void);
unsigned lp_b(void);

int main(void)
{
    return (int)(lp_a() + lp_b());
}
