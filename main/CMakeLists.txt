set(CUSTOM_SRCS
    main.c
    src/general.c
    src/button.c
    src/led.c
    src/lte.c
    src/json_maker.c
    src/heartbeat.c
    src/flash.c
    src/ir.cpp
    src/ble.c
)

set(includeDirs inc)

idf_component_register(
    SRCS ${CUSTOM_SRCS}
    INCLUDE_DIRS ${includeDirs}
    REQUIRES bt nvs_flash log custom_arduino esp_timer driver
)
