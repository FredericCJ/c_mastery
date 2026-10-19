# Saved-environment case sheet

Required X-T65-006-R1 cases. For each, identify the relevant draft clause,
missing assumptions, automatic-object rules and repair limits. These snippets
are reasoning inputs, not executable undefined-behavior tests. Assume declared
live jmp_buf `env`, same-thread calls unless the case changes that premise,
and standard declarations. Supply the full surrounding lifetime diagram.

* J1: `if (setjmp(env) == 0) { work(); }`. Contrast with
  `int rc = setjmp(env);` and `if (setjmp(env) + 1)`. Classify placement rather
  than relying on whether one compiler accepts translation.
* J2: in the destination function, `int local=0; if(setjmp(env)==0) {
  local=7; longjmp(env,1); } use(local);`. Do not assign a portable old/new value.
* J3: change J2 to `volatile int local`. State precisely which issue is fixed
  and which lifetime/thread/resource requirements are unaffected.
* J4: a helper saves env and returns; its caller later invokes longjmp. Contrast
  a caller-owned jmp_buf with a still-live destination function: they are not
  the same condition.
* J5: a second thread invokes longjmp on the first thread's saved environment.
  A mutex preventing simultaneous access does not change the destination rule.
* J6: `if(flag) { int a[n]; (void)a; setjmp(env); } else { longjmp(env,1); }`.
  Treat the schematic control flow as leaving the variably modified scope
  between save and restore; explain why merely keeping the enclosing function
  alive is insufficient. No executable trace is supplied.
* J7: a pointer parameter to a caller-owned context remains unchanged while a
  callee changes that context before jumping back. Contrast context storage
  with a modified automatic object local to the destination function.
* J8: acquire a resource and increment an ordinary external counter, then jump.
  Identify what longjmp restores, what still owns the resource and whether the
  counter has reverted. A cleanup callback is an explicit protocol operation.

The positive `tests/defined.c` actually uses switch, equality and unary-not
controlling expressions, an observable volatile local and longjmp value0.
Only its defined cases are compiled/executed as behavioral evidence. Reuse the
bounded PR-T12-04/T12.15 CONTROL contract as a prerequisite comparison, not an
assertion that its signal or thread experiments were rerun here. Published
C17/C23 access remains unavailable; N1570 is a C11 committee draft.
