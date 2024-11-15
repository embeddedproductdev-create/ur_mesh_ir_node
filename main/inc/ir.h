/*Global Variables*/
extern TaskHandle_t ir_recv_task_handle;

/*Function Declarations */
void ir_recv_intr_init();
void ir_recv_task(void *args);
void irRecvHandler(void *args);