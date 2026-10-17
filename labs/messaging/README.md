# Messaging laboratory

Complete the T73 projects and required exercises against include/messaging.h and CONTRACTS.md. src/messaging.c is a compiling scaffold with public initialization and unsolved operations. Public tests diagnose contract violations. The separate assessor tree is never copied into learner packages. Use cmake -S labs/messaging -B build/messaging-learner, build, then ctest --test-dir build/messaging-learner --output-on-failure. Unsolved anchors must fail by SC_CHECK, not undefined behavior.

Assessor campaigns require an explicit CMASTERY_ASSESSOR_ROOT and compile two distinct mutants per project. Tools/run_messaging.py records six compiler profiles; the declared separate TSan profile is required in the complete campaign. The pre-prose spike has exact plain/ASan/TSan logs. Actual execution does not discharge changed-condition, composition, independent assessment or cost obligations.
