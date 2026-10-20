# Instrument a target with gcov so the test run produces coverage data.
#
# Every project target is instrumented (not only the tests) so the files that
# no test exercises still show up in the report, with 0% coverage.

function(enable_coverage target_name)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target_name} PRIVATE --coverage -O0 -g)
    target_link_options(${target_name} PRIVATE --coverage)
  else()
    message(WARNING "Coverage is not supported for compiler '${CMAKE_CXX_COMPILER_ID}'")
  endif()
endfunction()
