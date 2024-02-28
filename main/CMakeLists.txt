set(srcs "main.cpp" "EC200U.c"
    "board.c")

idf_component_register(SRCS "${srcs}"
INCLUDE_DIRS  ".")
