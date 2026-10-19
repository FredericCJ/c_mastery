# T46.03/04 successor audit

Actual original criteria, CVM header/specification, dispatch/arithmetic/frame
implementation and old focused tests were read. Baseline arithmetic/call tests
exist; they do not provide wrapping semantics, a resumable API, host pending
identity, mutual/tail recursion, instruction-level invocation traces or actual
C-call instrumentation. This successor adds those mechanisms without editing the
retained provider. The named Nystrom primary narrative was inspected; source
inclusion markers were not expanded and are not claimed as read C listings.

Pilot231commands passed behavior in six native/two BE/four M4 profiles but
failed two O0 stack audits. Generated frames exposed whole-Vs compound-literal
copies. The implementation and test resets remove those copies; focused GCC/
Clang O0 M4 now run under the unchanged32768 reserve (conservative23768/22680
bytes before final trace edits). Final campaign must bind final source, not those
pilot numbers. Full T46 resource/store/provenance and T48 remain required. Physical,
independent/delayed learner, external review and final ISO access remain distinct.
