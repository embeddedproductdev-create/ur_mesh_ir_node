#ifndef BUTTON_H
#define BUTTON_H

#define ERROR_TAG "ERROR"
#define DEBUG_TAG "DEBUG"
#define BUTTON_TAG "BUTTON"

/*Global Variables*/
extern TaskHandle_t button_task_handle;

/*Function Declarations*/
void button_intr_init();
void button_task(void *args);

#endif