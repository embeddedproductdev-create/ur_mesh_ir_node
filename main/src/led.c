#include <unistd.h>

#include "led.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "lte.h"

static led_color_t current_color = {LOW, LOW, LOW};
static led_state_t current_state = LED_STATE_IDLE;
static esp_timer_handle_t blink_timer;

colors_t colors;

// Internal function prototypes
static void apply_color(led_color_t color);
static void led_blink_callback(void *arg);

void led_init(void) {
    colors.RED = (led_color_t){HIGH, LOW, LOW};
    colors.BLUE = (led_color_t){LOW, LOW, HIGH};
    colors.GREEN = (led_color_t){LOW, HIGH, LOW};
    colors.YELLOW = (led_color_t){LOW, HIGH, HIGH};
    colors.MAGENTA = (led_color_t){HIGH, LOW, HIGH};
    colors.ORANGE = (led_color_t){HIGH, HIGH, LOW};
    colors.WHITE = (led_color_t){HIGH, HIGH, HIGH};
    colors.OFF = (led_color_t){LOW, LOW, LOW};


    gpio_set_direction(LED_PIN_RED, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PIN_GREEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PIN_BLUE, GPIO_MODE_OUTPUT);

    update_led_status();

    // Configure blink timer
    esp_timer_create_args_t blink_timer_args = {
        .callback = led_blink_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "LED Blink Timer"
    };
    esp_timer_create(&blink_timer_args, &blink_timer);
}

const char* get_led_state_string(led_state_t state) {
    switch (state) {
        case LED_STATE_IDLE:
            return "LED_STATE_IDLE";
        case LED_STATE_UNCONFIGURED:
            return "LED_STATE_UNCONFIGURED";
        case LED_STATE_UNREGISTERED:
            return "LED_STATE_UNREGISTERED";
        case LED_STATE_MQTT_NOT_CONNECTED:
            return "LED_STATE_MQTT_NOT_CONNECTED";
        case LED_STATE_SENDING_IR_COMMAND:
            return "LED_STATE_SENDING_IR_COMMAND";
        case LED_STATE_TEACHING_MODE:
            return "LED_STATE_TEACHING_MODE";
        case LED_STATE_MQTT_CMD_RECVD:
            return "LED_STATE_MQTT_CMD_RECVD";
        default:
            return "UNKNOWN_STATE";
    }
}

void led_set_state(led_state_t state) {
    current_state = state;
    ESP_LOGI(LED_TAG, "Setting LED State : %s",get_led_state_string(state));
    esp_timer_stop(blink_timer);  // Stop any existing blink pattern

    switch (state) {
        case LED_STATE_IDLE:
            led_set_color(colors.GREEN);
            break;
        
        case LED_STATE_UNREGISTERED:
            current_color = colors.RED;
            esp_timer_start_periodic(blink_timer, SLOW_BLINK_INTERVAL_MS * 1000); // Toggle Red/Blue every 500 ms
            break;
        
        case LED_STATE_UNCONFIGURED:
            led_set_color(colors.BLUE);
            break;

        case LED_STATE_MQTT_NOT_CONNECTED:
            led_set_color(colors.RED);
            break;
        
        case LED_STATE_SENDING_IR_COMMAND:
            led_set_color(colors.MAGENTA);
            break;
        
        case LED_STATE_TEACHING_MODE:
            current_color = colors.BLUE; // Fast Blinking Blue
            esp_timer_start_periodic(blink_timer, FAST_BLINK_INTERVAL_MS * 1000);
            break;
        
        case LED_STATE_MQTT_CMD_RECVD:
            led_set_color(colors.ORANGE);
            sleep(1);
            update_led_status();
            break;

        default:
            led_set_color(colors.OFF);
            break;
    }
}

void update_led_status()
{
    if(teaching_in_progress) led_set_state(LED_STATE_TEACHING_MODE);
    else if(sending_ir_command) led_set_state(LED_STATE_SENDING_IR_COMMAND);
    else if(!mqtt_connected) led_set_state(LED_STATE_MQTT_NOT_CONNECTED);
    else if(!registered) led_set_state(LED_STATE_UNREGISTERED);
    else if(!configured) led_set_state(LED_STATE_UNCONFIGURED);
}

void led_set_color(led_color_t color) {
    current_color = color;
    if(LED_INVERTED){
        if(color.red) color.red = 0; else color.red = 1;
        if(color.blue) color.blue = 0; else color.blue = 1;
        if(color.green) color.green = 0; else color.green = 1;
    }
    apply_color(color);
}

static void apply_color(led_color_t color) {
    gpio_set_level(LED_PIN_RED, color.red);
    gpio_set_level(LED_PIN_GREEN, color.green);
    gpio_set_level(LED_PIN_BLUE, color.blue);
}

static void led_blink_callback(void *arg) {
    switch(current_state)
    {
        case LED_STATE_UNREGISTERED:
            if(current_color.red) led_set_color(colors.BLUE);
            else led_set_color(colors.RED);
            break;
        
        case LED_STATE_TEACHING_MODE:
            if(current_color.blue) led_set_color(colors.OFF);
            else led_set_color(colors.BLUE);

        default:
            break;
    }
}
