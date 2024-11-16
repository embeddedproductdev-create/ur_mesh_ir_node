

#define IR_TAG "IR"
#define IR_RECV_GPIO 38
#define IR_TRAN_GPIO 7

#define RECV_BUFFER_SIZE 1024
#define KTIMEOUT 50
#define SAVE_BUFFER_FLAG true

/*Global Variables*/
extern TaskHandle_t ir_recv_task_handle;

extern const char *RAW_IR_PROTOCOL;
extern const char *DAIKIN_IR_PROTOCOL;
extern const char *DAIKIN200_IR_PROTOCOL;
extern const char *DAIKIN216_IR_PROTOCOL;
extern const char *DAIKIN2_IR_PROTOCOL;
extern const char *DAIKIN160_IR_PROTOCOL;
extern const char *DAIKIN176_IR_PROTOCOL;
extern const char *DAIKIN64_IR_PROTOCOL;
extern const char *DAIKIN152_IR_PROTOCOL;
extern const char *DAIKIN128_IR_PROTOCOL;
extern const char *HITACHI_AC296_IR_PROTOCOL;
extern const char *HITACHI_AC_IR_PROTOCOL;
extern const char *HITACHI_AC1_IR_PROTOCOL;
extern const char *HITACHI_AC424_IR_PROTOCOL;
extern const char *HITACHI_AC344_IR_PROTOCOL;
extern const char *HITACHI_AC264_IR_PROTOCOL;
extern const char *VOLTAS_IR_PROTOCOL;
extern const char *SAMSUNG_AC_IR_PROTOCOL;
extern const char *HAIER_AC_IR_PROTOCOL;
extern const char *HAIER_AC176_IR_PROTOCOL;
extern const char *HAIER_AC160_IR_PROTOCOL;
extern const char *CARRIER_AC64_IR_PROTOCOL;
extern const char *LG2_IR_PROTOCOL;
extern const char *LG_IR_PROTOCOL;
extern const char *TOSHIBA_AC_IR_PROTOCOL;
extern const char *MITSUBISHI112_IR_PROTOCOL;
extern const char *MITSUBISHI136_IR_PROTOCOL;
extern const char *MITSUBISHI_AC_IR_PROTOCOL;
extern const char *MITSUBISHI_HEAVY_88_IR_PROTOCOL;
extern const char *MITSUBISHI_HEAVY_152_IR_PROTOCOL;
extern const char *UNKNOWN_IR_PROTOCOL;
extern const char *UNUSED_IR_PROTOCOL;
extern const char *INVALID_IR_PROTOCOL;


/*C - Function Declarations */

/*C++ - Function Declarations*/
#ifdef __cplusplus
extern "C" {
#endif

void ir_recv_task(void *args);
void ir_tran_setup();
const char *get_protocol_string(int16_t protocol);
bool is_supported_remote(int16_t protocol);
void ir_transmit();

#ifdef __cplusplus
}
#endif