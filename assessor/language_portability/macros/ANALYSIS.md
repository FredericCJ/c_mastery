# Macro diagnosis and construction key

T06.02.a: A→B→7. SELF's replacement finds its own name and suppresses it, leaving SELF+1; the declared object5 gives6. Argument prescan expands TWICE(3) before the outer TWICE sees it, yielding12. This is distinct from a name suppressed during a replacement scan.

T06.02.b: F/G without a following left-parenthesis token are function designators. The function returns13; F(3)/G(3) invoke the macro and yield4. (F)(3) shields the name from function-like invocation, not from the language's function-call semantics. Comment/space before ( does not stop invocation. Whitespace between H and ( in its definition makes H object-like, and its replacement (F) calls the real function.

T06.02.c: outer prescan expands PAIR to1,7, which becomes two arguments to ID on rescan. The repair deliberately substitutes inside parentheses before ID parsing. A compound literal's braces do not change macro argument nesting: ((Pair){3,4}) shields the complete expression, whereas (Pair){3,4} does not. Do not claim the parenthesized repair handles an arbitrary tuple or introduces evaluation ordering beyond the resulting comma expression.

T06.03.a: # and ## block argument prescan at their operands. An extra ordinary-substitution layer creates VALUE→7 first. Stringification normalizes only whitespace between preprocessing tokens; it preserves the token spellings with escaping needed to construct a string. The expected runtime content is alpha followed by space, quoted q-backslash-quote-x, space plus space beta.

T06.03.b: invalid paste is undefined preprocessing behavior in the inspected draft, so retain observed diagnostics without calling them mandatory. Name validation prevents that operation for admitted generator input. Empty macro arguments produce placemarkers; nonempty+placemarker retains the nonempty token and both-empty disappears before stringification. Generator empty-suffix refusal is a separate API decision.

T06.03.c: a valid item_key preprocessing token can still name incompatible declarations. Preprocessor success supplies no type-safety proof. Reserved identifier rules depend on spelling, scope, linkage and included headers; the generator uses a deliberately fixed application namespace and rejects all requested reserved-prefix inputs before output. No undefined reserved-name executable is needed for this audit.

The finite independent oracle contains7 result fields, not a full replacement algorithm. Review explanations and delayed changes separately. Required compiler negatives must fail for their named phase/cause. No learner mastery or independent human review follows from author execution.
