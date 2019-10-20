function(enable_warnings warning_target)
  if(NOT TARGET ${warning_target})
    message(WARNING "MediaElch warnings: ${warning_target} is not a target.")
    return()
  endif()

  if(MSVC)
    target_compile_options(${warning_target} PRIVATE /W3)

  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(
      ${warning_target}
      PRIVATE
        -Wall
        -Wextra
        -pedantic
        # Warnings that are not enabled but -Wall/-Wextra See
        # https://kristerw.blogspot.com/2017/09/useful-gcc-warning- options-not-
        # enabled.html
        -Wunknown-pragmas
        -Wundef
        -Wold-style-cast # warn for c-style casts (e.g. `(int) 3.0`)
        -Wno-useless-cast # deactivated because of moc
        -Wdisabled-optimization
        -Wstrict-overflow=4
        -Winit-self
        -Wpointer-arith
        -Wduplicated-cond
        -Wdouble-promotion
        -Wshadow # warn the user if a variable declaration shadows one from a
                 # parent context
        -Wduplicated-branches
        -Wrestrict
        -Wnull-dereference # warn if a null dereference is detected
        -Wlogical-op
        -Wunsafe-loop-optimizations
        -Wno-error=unsafe-loop-optimizations
        -Wformat=2
        -Wmissing-field-initializers
    )

  elseif(
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}"
                                                   STREQUAL "AppleClang"
  )
    target_compile_options(
      ${warning_target}
      PRIVATE
        -Wall -Wextra -pedantic -Wdocumentation # Warns about doxygen variable
                                                # name mismatches, etc.
    )

  else()
    message(
      WARNING
        "Unsupported compiler '${CMAKE_CXX_COMPILER_ID}'. It may not work."
    )
    target_compile_options(${warning_target} PRIVATE -Wall -Wextra -pedantic)

  endif()
endfunction()
