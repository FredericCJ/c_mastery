cmake_minimum_required(VERSION 3.20)
project(binary_inspection C)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_executable(inspect_records driver.c record.c)
target_compile_features(inspect_records PRIVATE c_std_17)
enable_testing()
foreach(stage stages bytes linking debugger cortex replay live)
    add_test(NAME inspection.${stage}
             COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/inspect_lab.py"
                     check "${stage}" --cc "${CMAKE_C_COMPILER}"
                     --receipt "${CMAKE_CURRENT_BINARY_DIR}/targets/inspection/${stage}.json")
    set_tests_properties(inspection.${stage} PROPERTIES TIMEOUT 100)
endforeach()
