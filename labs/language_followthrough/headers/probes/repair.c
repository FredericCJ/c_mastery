int hh_syntax(void) { return 0; }
int hh_conflict(int x) { return x; }
int hh_arity(int x, int y) { return x + y; }
int hh_pointer(const int *p) { return *p; }
int hh_repaired(void) { int x = 7; return hh_pointer(&x) + hh_arity(2,3); }
