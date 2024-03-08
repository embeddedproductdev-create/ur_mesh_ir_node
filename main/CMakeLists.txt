set(srcs "main.cpp"
"../src/Custom/LED.c"
"../src/Custom/button.c"
"../src/Custom/temperature_sensor.c"
"../src/LTE/LTE.c"
"../src/IR/main_IR.cpp"
"../src/Mesh/mesh_main.c"
"../src/Custom/accesspoint.cpp"
)

idf_component_register(SRCS "${srcs}"
INCLUDE_DIRS  ".")


