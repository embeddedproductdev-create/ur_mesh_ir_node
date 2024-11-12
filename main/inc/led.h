#ifndef LED_H
#define LED_H

#include <stdint.h>

// Fast and slow blink intervals
#define FAST_BLINK_INTERVAL_MS 250
#define SLOW_BLINK_INTERVAL_MS 500

// Define GPIO pins for RGB LED
#define LED_PIN_RED    21
#define LED_PIN_GREEN  14
#define LED_PIN_BLUE   13

#define LED_INVERTED 1

#if(LED_INVERTED)
#define HIGH 0
#define LOW 1
#endif

#if(!LED_INVERTED)
#define HIGH 1
#define LOW 0
#endif

#define LED_TAG "LED"

// Enum for LED states
typedef enum {
    LED_STATE_IDLE,
    LED_STATE_REGISTERED,
    LED_STATE_UNREGISTERED,
    LED_STATE_MQTT_NOT_CONNECTED,
    LED_STATE_SENDING_IR_COMMAND,
    LED_STATE_TEACHING_MODE
} led_state_t;

// Structure for RGB color
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} led_color_t;

void led_init(void);
void led_set_state(led_state_t state);
void led_set_color(led_color_t color);
void led_on(void);
void led_off(void);
void led_blink(void);

#endif
