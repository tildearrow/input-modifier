add_library(imodplugintest SHARED test.cpp)
set_target_properties(imodplugintest PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins)
