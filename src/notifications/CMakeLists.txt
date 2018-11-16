add_library(mediaelch_notifications OBJECT NotificationBox.cpp Notificator.cpp)

if(APPLE)
  target_sources(mediaelch_notifications PRIVATE MacNotificationHandler.mm)
endif()

target_link_libraries(mediaelch_notifications PRIVATE Qt5::Widgets)
mediaelch_post_target_defaults(mediaelch_notifications)
