set( CUSTOM_SRCS
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

idf_component_register(
    SRCS ${CUSTOM_SRCS}
    INCLUDE_DIRS "inc"
)
