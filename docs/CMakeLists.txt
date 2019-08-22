# ------------------------------------------------------------------------------
# Doxygen
find_package(Doxygen OPTIONAL_COMPONENTS dot)
if(DOXYGEN_FOUND)
  set(DOXYGEN_PROJECTNAME "MediaElch")
  set(DOXYGEN_PROJECT_BRIEF "${PROJECT_DESCRIPTION}")
  set(DOXYGEN_GENERATE_TREEVIEW YES)
  set(DOXYGEN_PROJECT_LOGO "MediaElch.png")
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ../README.md)
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_EXTRACT_PRIVATE NO)
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
