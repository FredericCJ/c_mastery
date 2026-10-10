foreach(case array list freelist radix dp filter)
 add_test(NAME algorithms.negative.${case} COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/v4_mutation.py" --cc "${CMAKE_C_COMPILER}" --source "${ENGINE_DIR}/v4_algo.c" --test "${CMAKE_CURRENT_SOURCE_DIR}/tests/v4_algo.c" --mutations "${ENGINE_DIR}/v4_mutations.json" --case ${case} --include "${CMAKE_CURRENT_SOURCE_DIR}/include" )
 set_tests_properties(algorithms.negative.${case} PROPERTIES TIMEOUT 30 LABELS "negative-control;v4")
endforeach()
