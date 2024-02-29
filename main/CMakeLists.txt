set(srcs "main.cpp"
"../src/Custom/LED.c"
"../src/Custom/temperature_sensor.c"
"../src/LTE/LTE.c"
"../src/IR/main_IR.cpp"
)

idf_component_register(SRCS "${srcs}"
INCLUDE_DIRS  ".")


