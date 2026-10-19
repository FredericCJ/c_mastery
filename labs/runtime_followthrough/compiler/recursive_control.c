/* Original AST-auditor sensitivity fixture, never a compiler implementation. */
unsigned vc_recursive_control(unsigned n) {
  return n ? 1u + vc_recursive_control(n - 1u) : 0u;
}
