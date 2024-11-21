#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <lte.h>

void hb_timer_init();
void hb_timer_start();
void hb_timer_stop();
void hb_timer_restart();
void hb_init();
void handle_setting_hb_publish_configuration(CommandStruct *cmd_struct);

#endif