#ifndef HEARTBEAT_H
#define HEARTBEAT_H

void hb_timer_init();
void hb_timer_start();
void hb_timer_stop();
void hb_timer_restart();
void hb_init();
void handle_setting_hb_publish_configuration(uint16_t publishPeriod);

#endif