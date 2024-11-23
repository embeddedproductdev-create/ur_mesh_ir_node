cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/bluetooth/esp_ble_mesh/common_components/fast_provisioning)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(IR_BLE_MESH)
