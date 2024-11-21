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
    src/ble_prov.c
    src/ble_node.c
)

set(includeDirs inc)

idf_component_register(
    SRCS ${CUSTOM_SRCS}
    INCLUDE_DIRS ${includeDirs}
)
