# ------------------------------------------------------------------------------
# Doxygen
find_package(Doxygen OPTIONAL_COMPONENTS dot)
if(DOXYGEN_FOUND)
  set(DOXYGEN_PROJECTNAME "${PROJECT_NAME}")
  set(DOXYGEN_PROJECT_BRIEF "${PROJECT_DESCRIPTION}")
  set(DOXYGEN_GENERATE_TREEVIEW YES)
  set(DOXYGEN_PROJECT_LOGO "MediaElch.png")
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ../README.md)
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_EXTRACT_PRIVATE YES)
  set(DOXYGEN_EXTRACT_STATIC YES)
  set(DOXYGEN_EXTRACT_LOCAL_CLASSES YES)
  set(DOXYGEN_GENERATE_TODOLIST YES)
  set(GENERATE_QHP YES)
  set(QCH_FILE "MediaElch.qch")
  set(QHP_NAMESPACE "org.doxygen.MediaElch")
  set(UML_LOOK YES)

  doxygen_add_docs(
    doxygen ../README.md ../src COMMENT "Generate doxygen documentation"
  )

else()
  message(
    "=> Doxygen needs to be installed to generate developer documentation"
  )

endif()

# ------------------------------------------------------------------------------
# Sphinx User Documentation
find_package(Sphinx)
if(SPHINX_FOUND)
  add_custom_target(
    docs
    COMMAND
      ${SPHINX_EXECUTABLE} -M html
      "${CMAKE_CURRENT_SOURCE_DIR}/user/docs/source"
      "${CMAKE_CURRENT_BINARY_DIR}/user"
    COMMAND
      ${CMAKE_COMMAND} -E echo
      "User documentation available at '${CMAKE_CURRENT_BINARY_DIR}/user/html/index.html'"
    COMMENT "Build HTML documentation with Sphinx"
  )

else()
  message("=> Sphinx needs to be installed to generate user documentation")

endif()
