include(FindPkgConfig)

pkg_check_modules(DBUS REQUIRED dbus-1)

if (PKG_CONFIG_FOUND)
  if (DBUS_FOUND)
    add_library(imodpluginrazer SHARED dbus.cpp razer.cpp)
    set_target_properties(imodpluginrazer PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins)
    include_directories(${DBUS_INCLUDE_DIRS})
    #target_link_libraries(imodpluginrazer $(DBUS_
  endif()
endif()

