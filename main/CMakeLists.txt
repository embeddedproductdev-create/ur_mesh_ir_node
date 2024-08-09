set(srcs "main.cpp"
"../src/JSON/json_maker.c"
"../src/Custom/LED.c"
"../src/flash/flash.c"
"../src/Custom/printf_custom.c"
"../src/Custom/button.cpp"
"../src/Custom/temperature_sensor.c"
"../src/LTE/LTE.cpp"
"../src/LTE/Cloud.c"
"../src/IR/main_IR.cpp"
"../src/Mesh/gwy_mesh_main.c"
"../src/Mesh/node_mesh_main.c"
"../src/Custom/queue.cpp"
"../src/Custom/accesspoint.cpp"
"../src/Custom/esp_insights.cpp"
)

idf_component_register(SRCS "${srcs}"
INCLUDE_DIRS  ".")


