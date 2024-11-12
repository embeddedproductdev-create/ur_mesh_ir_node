set( CUSTOM_SRCS
    main.c
    src/general.c
    src/button.c
)

idf_component_register(
    SRCS ${CUSTOM_SRCS}
    INCLUDE_DIRS "inc"
)
