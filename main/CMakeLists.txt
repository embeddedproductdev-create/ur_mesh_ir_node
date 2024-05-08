set(srcs "main.cpp"
"../src/Custom/LED.c"
"../src/flash/flash.c"
"../src/Custom/printf_custom.c"
"../src/Custom/button.cpp"
"../src/Custom/temperature_sensor.c"
"../src/LTE/LTE.c"
"../src/LTE/Cloud.c"
"../src/IR/main_IR.cpp"
"../src/Mesh/gwy_mesh_main.c"
"../src/Mesh/node_mesh_main.c"
"../src/Custom/queue.c"
"../src/Custom/accesspoint.cpp"
)

idf_component_register(SRCS "${srcs}"
INCLUDE_DIRS  ".")


