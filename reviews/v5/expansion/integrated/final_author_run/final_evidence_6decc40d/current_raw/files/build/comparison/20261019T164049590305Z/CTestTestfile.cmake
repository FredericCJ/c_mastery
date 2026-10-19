# CMake generated Testfile for 
# Source directory: /mnt/f/Learning/Curricula/c_mastery/labs/os_observatory
# Build directory: /mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[os_schedulers]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_tests" "schedulers")
set_tests_properties([=[os_schedulers]=] PROPERTIES  TIMEOUT "10" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;17;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_fairness]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_tests" "fairness")
set_tests_properties([=[os_fairness]=] PROPERTIES  TIMEOUT "10" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;17;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_clock]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_tests" "clock")
set_tests_properties([=[os_clock]=] PROPERTIES  TIMEOUT "10" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;17;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_process]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_tests" "process")
set_tests_properties([=[os_process]=] PROPERTIES  TIMEOUT "10" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;17;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_thread]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_tests" "thread")
set_tests_properties([=[os_thread]=] PROPERTIES  TIMEOUT "10" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;17;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_descriptor]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_tests" "descriptor")
set_tests_properties([=[os_descriptor]=] PROPERTIES  TIMEOUT "10" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;17;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_signal]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_tests" "signal")
set_tests_properties([=[os_signal]=] PROPERTIES  TIMEOUT "10" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;17;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_memory]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_tests" "memory")
set_tests_properties([=[os_memory]=] PROPERTIES  TIMEOUT "10" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;17;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_io]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_tests" "io")
set_tests_properties([=[os_io]=] PROPERTIES  TIMEOUT "10" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;17;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_linux_comparison]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_linux_compare")
set_tests_properties([=[os_linux_comparison]=] PROPERTIES  TIMEOUT "15" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;25;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_mutations]=] "/usr/bin/python3" "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/tests/mutations.py" "--compiler" "/usr/bin/cc" "--impl" "/mnt/f/Learning/Curricula/c_mastery/assessor/os_observatory" "--lab" "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory")
set_tests_properties([=[os_mutations]=] PROPERTIES  _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;27;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
add_test([=[os_freertos_comparison]=] "/mnt/f/Learning/Curricula/c_mastery/build/comparison/20261019T164049590305Z/os_freertos_compare")
set_tests_properties([=[os_freertos_comparison]=] PROPERTIES  TIMEOUT "15" _BACKTRACE_TRIPLES "/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;37;add_test;/mnt/f/Learning/Curricula/c_mastery/labs/os_observatory/CMakeLists.txt;0;")
