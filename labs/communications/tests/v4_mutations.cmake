foreach(case reassembly arp tcp_fault canfd)
 add_test(NAME communications.negative.${case} COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/v4_mutation.py" --cc "${CMAKE_C_COMPILER}" --source "${COM_SOURCE}/v4_com.c" --test "${CMAKE_CURRENT_SOURCE_DIR}/tests/v4_com.c" --mutations "${COM_SOURCE}/v4_mutations.json" --case ${case} --include "${CMAKE_CURRENT_SOURCE_DIR}/include" --extra "${COM_SOURCE}/http.c" --extra "${COM_SOURCE}/edu7.c" --extra "${COM_SOURCE}/net.c" --extra "${COM_SOURCE}/mcu.c")
 set_tests_properties(communications.negative.${case} PROPERTIES TIMEOUT 30 LABELS "negative-control;v4")
endforeach()
