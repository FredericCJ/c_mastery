# Withheld transfer packet

Use original `AX-PR-T02-03-...-DELAYED` identities and minimum seven-day delay.
Record actual timestamps, learner artifacts, permitted materials and assessor
observations. Author reference runs do not fill these fields.

1. T02.09.a: Add a borrowed pointer initialized to a live int, then reset. Derive
   null value semantics and give an explicit representation countermodel to zeroed
   bytes; do not claim that countermodel was physically executed.
2. T02.09.b: Move the designated literal to file scope, obtain two aliases, mutate
   index2 through one and explain the other's observation and lifetime. Then discuss
   which conclusion would change for a const-qualified literal.
3. T02.09.c: Return a FvView by value containing a pointer into a local literal.
   Classify without calling it; repair using caller storage and identify all aliases
   invalidated at each owner's eventual lifetime end.
4. T02.10.a: Consume only an opaque owner handle, acquire both const and mutable
   aliases, and demonstrate allowed owner mutation. Explain precisely what the
   header prevents and what remains programmer discipline.
5. T02.10.b: Keep a live view and a detached snapshot across owner update/reset.
   Supply distinct expected observations and short-capacity rollback tests; then
   end the owner and classify the live view without evaluating its expired pointer.
6. T02.10.c: Change copy admission to permit overlapping ranges. Implement the
   one-base/extent/offset operation, verify both directions, equal positions, zero
   count at one-past, and wrapped-size attempts against saved original values.
   Explain why volatile and restrict still supply no thread synchronization.

For every row require a contract derivation, applicable implementation or rigorous
reasoning-only analysis, an independent falsification case, and evidence limits.
The supplied reference, model, diagnostic controls and oracle implement the
assessor variants now; actual delayed attempts remain pending.
