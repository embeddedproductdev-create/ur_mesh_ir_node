#include "led.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

static led_color_t current_color = {LOW, LOW, LOW};
static led_state_t current_state = LED_STATE_IDLE;
static esp_timer_handle_t blink_timer;
static bool led_is_on = false;

// Internal function prototypes
static void apply_color(led_color_t color);
static void led_blink_callback(void *arg);

void led_init(void) {
    gpio_set_direction(LED_PIN_RED, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PIN_GREEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PIN_BLUE, GPIO_MODE_OUTPUT);
    
    led_off();

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
        case LED_STATE_REGISTERED:
            return "LED_STATE_REGISTERED";
        case LED_STATE_UNREGISTERED:
            return "LED_STATE_UNREGISTERED";
        case LED_STATE_MQTT_NOT_CONNECTED:
            return "LED_STATE_MQTT_NOT_CONNECTED";
        case LED_STATE_SENDING_IR_COMMAND:
            return "LED_STATE_SENDING_IR_COMMAND";
        case LED_STATE_TEACHING_MODE:
            return "LED_STATE_TEACHING_MODE";
        default:
            return "UNKNOWN_STATE";
    }
}

void led_set_state(led_state_t state) {
    ESP_LOGI(LED_TAG, "Setting LED State : %s",get_led_state_string(state));
    current_state = state;
    esp_timer_stop(blink_timer);  // Stop any existing blink pattern
    led_off(); // Turn off LED before applying new state

    switch (state) {
        case LED_STATE_IDLE:
            current_color = (led_color_t){LOW, HIGH, LOW}; // Solid Green
            led_on();
            break;
        
        case LED_STATE_UNREGISTERED:
            current_color = (led_color_t){HIGH, LOW, LOW};
            esp_timer_start_periodic(blink_timer, SLOW_BLINK_INTERVAL_MS * 1000); // Toggle Red/Blue every 500 ms
            break;
        
        case LED_STATE_REGISTERED:
            current_color = (led_color_t){LOW, LOW, HIGH}; // Solid Blue
            led_on();
            break;

        case LED_STATE_MQTT_NOT_CONNECTED:
            current_color = (led_color_t){HIGH, LOW, LOW}; // Solid Red
            led_on();
            break;
        
        case LED_STATE_SENDING_IR_COMMAND:
            current_color = (led_color_t){HIGH, LOW, HIGH}; // Purple
            led_on();
            break;
        
        case LED_STATE_TEACHING_MODE:
            current_color = (led_color_t){LOW, LOW, HIGH}; // Fast Blinking Blue
            esp_timer_start_periodic(blink_timer, FAST_BLINK_INTERVAL_MS * 1000);
            break;

        default:
            led_off(); // For any other unimplemented state, turn off LED
            break;
    }
}

void led_set_color(led_color_t color) {
    current_color = color;
    apply_color(current_color);
}

void led_on(void) {
    apply_color(current_color);
}

void led_off(void) {
    gpio_set_level(LED_PIN_RED, LOW);
    gpio_set_level(LED_PIN_GREEN, LOW);
    gpio_set_level(LED_PIN_BLUE, LOW);
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
            if(current_color.red) {
                led_set_color((led_color_t){LOW, LOW, HIGH});
                current_color.red = 0;
            }
            else {
                led_set_color((led_color_t){HIGH, LOW, LOW});
                current_color.blue = 0;
            }
            break;
        default:
            break;
    }
    led_is_on = !led_is_on;
}
