#define IR_TAG "IR"
#define IR_RECV_GPIO 38
#define IR_TRAN_GPIO 7

#define RECV_BUFFER_SIZE 1024
#define KTIMEOUT 50
#define SAVE_BUFFER_FLAG true

/*Global Variables*/
extern TaskHandle_t ir_recv_task_handle;

/*C - Function Declarations */
void ir_init();
void ir_recv_intr_init();
void ir_recv_task(void *args);
void irRecvHandler(void *args);
void ir_receiver_setup();

/*C++ - Function Declarations*/
#ifdef __cplusplus
extern "C" {
#endif

void ir_recv_configure();
void ir_tran_setup();
char *get_protocol_string(uint16_t protocol);
bool is_supported_remote(uint16_t protocol);
void decode_ir();

#ifdef __cplusplus
}
#endif