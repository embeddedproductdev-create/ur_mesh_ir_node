# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Add Insights components. Not required if espressif/esp_insights is included via IDF component manager */
if(DEFINED ENV{INSIGHTS_PATH})
  set(INSIGHTS_PATH $ENV{INSIGHTS_PATH})
else()
  set(INSIGHTS_PATH ${CMAKE_CURRENT_LIST_DIR})
endif(DEFINED ENV{INSIGHTS_PATH})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(PROJECT_VER "0.8.8")
project(IR_BLE_mesh)
