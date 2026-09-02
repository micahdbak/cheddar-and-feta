if(WIN32)
  set(BUILD_SHARED_LIBS ON)
  set(SDL3_DISABLE_WINDOWS_GAMEINPUT TRUE CACHE BOOL "" FORCE)
else()
  set(BUILD_SHARED_LIBS OFF)
  set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()

add_subdirectory(deps/libdatachannel EXCLUDE_FROM_ALL)
add_subdirectory(deps/SDL EXCLUDE_FROM_ALL)

add_library(engine
  src/audio_playback.cc
  src/bmp_texture.cc
  src/controller.cc
  src/font.cc
  src/game_const.cc
  src/game_draw.cc
  src/game_sprite.cc
  src/game_step.cc
  src/main.cc
  src/map.cc
  src/net_agent.cc
  src/net_connection.cc
  src/net_signaller.cc
  src/save_data.cc
  src/sprite.cc
)
target_include_directories(engine PUBLIC src)
target_link_libraries(engine LibDataChannel::LibDataChannel SDL3::SDL3)

if(WIN32)
  add_custom_command(TARGET engine POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_RUNTIME_DLLS:engine>
          $<TARGET_FILE_DIR:engine>
      COMMAND_EXPAND_LISTS
  )
endif()
