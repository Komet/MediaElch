# From https://medium.com/@alasher/colored-c-compiler-output-with-ninja-clang-
# gcc-10bfe7f2b949
if(${ENABLE_COLOR_OUTPUT})
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
  elseif(
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}"
                                                   STREQUAL "AppleClang"
  )
    add_compile_options(-fcolor-diagnostics)
  endif()
endif()
