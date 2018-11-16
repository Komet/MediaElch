if(ENABLE_CLANG_TIDY)
  message(STATUS "clang-tidy enabled")
  set(CMAKE_CXX_CLANG_TIDY clang-tidy -format-style=file)

elseif(ENABLE_CLANG_TIDY_FIX)
  message(STATUS "clang-tidy with auto-fix enabled")
  set(CMAKE_CXX_CLANG_TIDY clang-tidy -format-style=file -fix)

endif()
