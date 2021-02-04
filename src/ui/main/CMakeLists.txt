add_library(
  mediaelch_ui_main OBJECT
  AboutDialog.cpp
  FileScannerDialog.cpp
  MainWindow.cpp
  Message.cpp
  MyIconFont.cpp
  Navbar.cpp
  Update.cpp
  QuickOpen.cpp
)

target_link_libraries(
  mediaelch_ui_main PRIVATE Qt5::Widgets Qt5::MultimediaWidgets Qt5::Sql
)
mediaelch_post_target_defaults(mediaelch_ui_main)
