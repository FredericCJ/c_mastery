foreach(case startup vectors gpio uart spi i2c timer adc dma ethernet rs485 watchdog)
 add_test(NAME firmware_io.negative.${case} COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/v4_mutation.py" --cc "${CMAKE_C_COMPILER}" --source "${FW_SOURCE}/firmware_io.c" --test "${CMAKE_CURRENT_SOURCE_DIR}/tests/contracts.c" --mutations "${FW_SOURCE}/v4_mutations.json" --case ${case} --include "${CMAKE_CURRENT_SOURCE_DIR}/include" )
 set_tests_properties(firmware_io.negative.${case} PROPERTIES TIMEOUT 30 LABELS "negative-control;v4")
endforeach()
