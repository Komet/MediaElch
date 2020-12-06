add_library(
  mediaelch_ui_smallwidgets OBJECT
  AlphabeticalList.cpp
  Badge.cpp
  ClosableImage.cpp
  FilterWidget.cpp
  ImageGallery.cpp
  ImageLabel.cpp
  LoadingStreamDetails.cpp
  LanguageCombo.cpp
  MediaFlags.cpp
  MessageLabel.cpp
  MusicTreeView.cpp
  MyCheckBox.cpp
  MyLabel.cpp
  MyLineEdit.cpp
  MySpinBox.cpp
  MySplitter.cpp
  MySplitterHandle.cpp
  MyTableView.cpp
  MyTableWidget.cpp
  MyTableWidgetItem.cpp
  MyTreeView.cpp
  MyWidget.cpp
  SearchOverlay.cpp
  SlidingStackedWidget.cpp
  TagCloud.cpp
  TvShowTreeView.cpp
)

target_link_libraries(
  mediaelch_ui_smallwidgets PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network
)
mediaelch_post_target_defaults(mediaelch_ui_smallwidgets)
