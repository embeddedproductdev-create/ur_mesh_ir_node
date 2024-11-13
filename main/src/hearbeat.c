#include "esp_timer.h"
#include "esp_log.h"

#include "../inc/lte.h"
#include "../inc/main.h"

static esp_timer_handle_t hb_timer_handle;

static void hb_callback(void *arg) 
{
    char *buffer = (char *)malloc(1024);
    struct jWriteControl *jwc = (struct jWriteControl *)malloc(sizeof(struct jWriteControl));
    jwOpen(jwc, buffer, 1024, JW_OBJECT, 1);
    jwObj_int(jwc, JSON_PACKET_ID_KEY, cmd_struct->packetid);
    jwObj_int(jwc, MSG_SEQ_NO_KEY, cmd_struct->msgseqno);
    jwObj_int(jwc, ERROR_CODE_KEY, cmd_struct.errorcode);
    jwClose(jwc);
    enqueue_for_publish(buffer);
}

/**
 * @brief Function that stops the hb timer
 * 
 */
void hb_timer_stop()
{
    esp_timer_stop(hb_timer_handle);
}

/**
 * @brief Function that starts the hb timer
 * 
 */
void hb_timer_start()
{
    esp_timer_start_periodic(hb_timer_handle, publishPeriod);
}

/**
 * @brief Function that initializes the heartbeat timer (but doesn't start)
 * 
 */
void hb_init()
{
    esp_timer_create_args_t hb_timer_args = {
        .callback = hb_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "LED Blink Timer"
    };
    esp_timer_create(&hb_timer_args, &hb_timer_handle);
}