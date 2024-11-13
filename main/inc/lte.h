/*Global Variables*/
extern TaskHandle_t lte_task_handle;

/*Function Declarations*/
void lte_task(void *args);
int8_t check_response(char *uart_data, const char *check_string);
int8_t fetch_and_check_data(bool logging, uint16_t timeout_ms, const char *check_string, const char *cmd_name);
int8_t send_cmd_and_check_response(bool logging, const char *cmd, const char *cmdName, const char *check_string, uint32_t timeout_ms);
void power_cycle_lte();