add_library(
  mediaelch_ui_notifications OBJECT NotificationBox.cpp Notificator.cpp
)

if(APPLE)
  target_sources(mediaelch_ui_notifications PRIVATE MacNotificationHandler.mm)
  set_source_files_properties(
    MacNotificationHandler.mm PROPERTIES COMPILE_OPTIONS
                                         "-fno-objc-msgsend-selector-stubs"
  )
endif()

target_link_libraries(
  mediaelch_ui_notifications PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)
mediaelch_post_target_defaults(mediaelch_ui_notifications)
