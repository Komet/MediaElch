if(NOT USE_EXTERN_QUAZIP)
  message(STATUS "Using QuaZip from submodule")
  set(QUAZIP_QT_MAJOR_VERSION
      ${QT_VERSION_MAJOR}
      CACHE STRING "Qt version to use (4, 5 or 6), defaults to 5" FORCE
  )
  add_subdirectory(quazip)
endif()
