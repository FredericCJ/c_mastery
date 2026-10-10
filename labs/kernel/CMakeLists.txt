cmake_minimum_required(VERSION 3.20)
project(c_mastery_kernel C)
enable_testing()
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMASTERY_ASSESSOR_ROOT "" CACHE PATH "Explicit opt-in assessor/kernel directory")
if(CMASTERY_ASSESSOR_ROOT)
  if(NOT IS_ABSOLUTE "${CMASTERY_ASSESSOR_ROOT}")
    message(FATAL_ERROR "CMASTERY_ASSESSOR_ROOT must be absolute")
  endif()
  set(CMK_SOURCE "${CMASTERY_ASSESSOR_ROOT}")
else()
  set(CMK_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/learner")
endif()
add_library(cmk_model STATIC "${CMK_SOURCE}/model.c")
target_include_directories(cmk_model PUBLIC include)
target_compile_options(cmk_model PRIVATE -Wall -Wextra -Wpedantic)
add_executable(cmk_contracts tests/contracts.c)
target_link_libraries(cmk_contracts PRIVATE cmk_model)
foreach(kind boot frames elf archive handles ranges)
  add_test(NAME kernel_${kind} COMMAND cmk_contracts ${kind})
  set_tests_properties(kernel_${kind} PROPERTIES LABELS "host;kernel")
endforeach()
option(CMASTERY_KERNEL_GUEST "Build/test the real x86-64 guest when required tools exist" ON)
foreach(dep RUNTIME_ROOT RUNTIME_INCLUDE RUNTIME_ADAPTERS STORAGE_ROOT STORAGE_INCLUDE)
  set(CMASTERY_${dep} "$ENV{CMASTERY_${dep}}" CACHE PATH "Explicit cross-family ${dep}")
  if(CMASTERY_${dep} AND NOT IS_ABSOLUTE "${CMASTERY_${dep}}")
    message(FATAL_ERROR "CMASTERY_${dep} must be absolute")
  endif()
endforeach()
find_package(Python3 COMPONENTS Interpreter REQUIRED)
if(CMASTERY_ASSESSOR_ROOT)
  add_test(NAME kernel_mutations COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mutations.py" --source "${CMK_SOURCE}"
    --lab "${CMAKE_CURRENT_SOURCE_DIR}" --cc "${CMAKE_C_COMPILER}")
  set_tests_properties(kernel_mutations PROPERTIES LABELS "host;kernel;mutation")
endif()
find_program(CMK_GCC gcc)
find_program(CMK_GRUB grub-mkrescue)
find_program(CMK_XORRISO xorriso)
find_program(CMK_QEMU qemu-system-x86_64)
if(CMASTERY_KERNEL_GUEST AND CMK_GCC AND CMK_GRUB AND CMK_XORRISO AND CMK_QEMU)
  add_custom_target(kernel_guest ALL COMMAND ${CMAKE_COMMAND} -E env
    "CMASTERY_RUNTIME_ROOT=${CMASTERY_RUNTIME_ROOT}"
    "CMASTERY_RUNTIME_INCLUDE=${CMASTERY_RUNTIME_INCLUDE}"
    "CMASTERY_RUNTIME_ADAPTERS=${CMASTERY_RUNTIME_ADAPTERS}"
    "CMASTERY_STORAGE_ROOT=${CMASTERY_STORAGE_ROOT}"
    "CMASTERY_STORAGE_INCLUDE=${CMASTERY_STORAGE_INCLUDE}"
    ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/guest.py" build
    --lab "${CMAKE_CURRENT_SOURCE_DIR}" --source "${CMK_SOURCE}" --build "${CMAKE_CURRENT_BINARY_DIR}/guest"
    VERBATIM)
  add_test(NAME kernel_guest COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/guest.py" test
    --lab "${CMAKE_CURRENT_SOURCE_DIR}" --source "${CMK_SOURCE}" --build "${CMAKE_CURRENT_BINARY_DIR}/guest")
  add_test(NAME kernel_boot_smoke COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/guest.py" smoke
    --lab "${CMAKE_CURRENT_SOURCE_DIR}" --source "${CMK_SOURCE}" --build "${CMAKE_CURRENT_BINARY_DIR}/guest")
  set_tests_properties(kernel_boot_smoke PROPERTIES TIMEOUT 15 RUN_SERIAL TRUE LABELS "qemu;kernel;boot")
  set_tests_properties(kernel_guest PROPERTIES TIMEOUT 90 RUN_SERIAL TRUE LABELS "qemu;kernel;integration")
  if(CMASTERY_RUNTIME_ROOT AND CMASTERY_RUNTIME_INCLUDE)
    add_test(NAME kernel_runtime COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tools/guest.py" test --require runtime
      --lab "${CMAKE_CURRENT_SOURCE_DIR}" --source "${CMK_SOURCE}" --build "${CMAKE_CURRENT_BINARY_DIR}/guest")
    set_tests_properties(kernel_runtime PROPERTIES TIMEOUT 90 RUN_SERIAL TRUE LABELS "qemu;kernel;runtime")
  endif()
  if(CMASTERY_STORAGE_ROOT AND CMASTERY_STORAGE_INCLUDE AND CMASTERY_RUNTIME_ROOT AND CMASTERY_RUNTIME_INCLUDE AND CMASTERY_RUNTIME_ADAPTERS)
    add_test(NAME kernel_integration COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tools/guest.py" test --require storage
      --lab "${CMAKE_CURRENT_SOURCE_DIR}" --source "${CMK_SOURCE}" --build "${CMAKE_CURRENT_BINARY_DIR}/guest")
    set_tests_properties(kernel_integration PROPERTIES TIMEOUT 90 RUN_SERIAL TRUE LABELS "qemu;kernel;storage;runtime")
  endif()
else()
  message(STATUS "Kernel guest unavailable or disabled: requires Linux gcc, GRUB, xorriso and QEMU; host passes do not close guest gates")
endif()
