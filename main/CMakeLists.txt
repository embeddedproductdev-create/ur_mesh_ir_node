set(srcs "main.cpp"
"../src/Custom/LED.c"
"../src/Custom/temperature_sensor.c"
"../src/LTE/LTE.c"
"../src/IR/main_IR_recv.cpp"
"../src/IR/main_IR_transmit.cpp"
)

idf_component_register(SRCS "${srcs}"
INCLUDE_DIRS  ".")


