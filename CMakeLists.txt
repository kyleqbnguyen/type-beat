cmake_minimum_required(VERSION 3.22)

project(
  type-beat
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/W4 /WX /permissive-)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_library(core OBJECT src/core/file_utils.cpp src/core/ffmpeg.cpp)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(core PUBLIC Qt6::Core)

add_library(settings OBJECT src/settings/app_settings.cpp)
target_include_directories(settings PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(settings PUBLIC Qt6::Core)

add_library(ui OBJECT src/ui/file_input.cpp src/ui/main_window.cpp)
target_include_directories(ui PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ui PUBLIC Qt6::Widgets Qt6::Core core settings)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ui core settings Qt6::Widgets
                                              Qt6::Core)

if(APPLE)
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES OUTPUT_NAME "TypeBeat"
               MACOSX_BUNDLE TRUE
               MACOSX_BUNDLE_BUNDLE_NAME "TypeBeat"
               MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
               MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
               MACOSX_BUNDLE_GUI_IDENTIFIER "com.typebeat.app"
               MACOSX_BUNDLE_INFO_PLIST
               "${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist.in")
endif()

if(WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES 
                        OUTPUT_NAME "TypeBeat"
                        WIN32_EXECUTABLE TRUE)
endif()

if(UNIX AND NOT APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "TypeBeat")
endif()

include(GNUInstallDirs)

if(APPLE)
  # macOS: Install the .app bundle
  install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION .)
elseif(WIN32)
  # Windows: Install executable to bin/
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

  # Install Qt runtime DLLs, FFmpeg binaries, and Qt plugin directories
  # deployed by windeployqt alongside the executable during CI builds.
  install(
    CODE "
    # Install all DLLs (Qt6Core.dll, Qt6Widgets.dll, Qt6Gui.dll, etc.)
    file(GLOB _deploy_dlls
         \"${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/*.dll\")
    foreach(_dll \${_deploy_dlls})
      file(INSTALL \${_dll} DESTINATION \"\${CMAKE_INSTALL_PREFIX}\")
    endforeach()

    # Install FFmpeg binaries (ffmpeg.exe, ffprobe.exe)
    file(GLOB _deploy_exes
         \"${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/ff*.exe\")
    foreach(_exe \${_deploy_exes})
      file(INSTALL \${_exe} DESTINATION \"\${CMAKE_INSTALL_PREFIX}\")
    endforeach()

    # Install Qt plugin directories (platforms, styles, imageformats, etc.)
    foreach(_dir platforms styles imageformats tls networkinformation)
      if(IS_DIRECTORY
         \"${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/\${_dir}\")
        file(INSTALL
             \"${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/\${_dir}\"
             DESTINATION \"\${CMAKE_INSTALL_PREFIX}\")
      endif()
    endforeach()
  ")
else()
  # Linux: Standard FHS layout
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(FILES packaging/linux/TypeBeat.desktop
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
  install(FILES packaging/linux/TypeBeat.png
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps)
  install(FILES packaging/linux/com.typebeat.app.appdata.xml
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
endif()

set(CPACK_PACKAGE_NAME "TypeBeat")
set(CPACK_PACKAGE_VENDOR "TypeBeat")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Combine visuals with audio to create videos")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "TypeBeat")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

if(WIN32)
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_NSIS_DISPLAY_NAME "TypeBeat")
  set(CPACK_NSIS_PACKAGE_NAME "TypeBeat")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/packaging/windows/TypeBeat.ico")
  set(CPACK_NSIS_MUI_UNIICON
      "${CMAKE_SOURCE_DIR}/packaging/windows/TypeBeat.ico")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "TypeBeat.exe")
  set(CPACK_NSIS_CREATE_ICONS_EXTRA
      "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\TypeBeat.lnk' '$INSTDIR\\\\TypeBeat.exe'"
  )
  set(CPACK_NSIS_DELETE_ICONS_EXTRA
      "Delete '$SMPROGRAMS\\\\$START_MENU\\\\TypeBeat.lnk'")
elseif(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_DMG_VOLUME_NAME "TypeBeat")
  set(CPACK_DMG_FORMAT "UDZO")
else()
  set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
