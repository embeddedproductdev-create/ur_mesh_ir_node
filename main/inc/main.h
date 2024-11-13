/**
 * ===================================================================
 * @brief VERY IMPORTANT MACROS !!!
 * DO NOT CHANGE THE THINGS BELOW WITHOUT KNOWING WHAT YOU ARE DOING
 * IT AFFECTS ACROSS THE WHOLE CODEBASE. YOU HAVE BEEN WARNED !!!
 * ===================================================================
 */
#define IS_GWY true
#define CLIENT_RELEASE false
/*====================================================================*/

#if(IS_GWY)
#define MAJ_VERSION 0
#define MIN_VERSION 8
#define PATCH_VERSION 9
#endif

#if(!IS_GWY)
#define MAJ_VERSION 0
#define MIN_VERSION 8
#define PATCH_VERSION 9
#endif


#if(!CLIENT_RELEASE)
#define MQTT_SERVER_IP "54.215.188.103"
#define MQTT_PORT 1883
#define MQTT_BROKER_USERNAME "QmaxSystems"
#define MQTT_BROKER_PASSWORD "Qmax_mosquitto_!@#"
#endif

#if(CLIENT_RELEASE)
#define MQTT_SERVER_IP "3.7.8.183"
#define MQTT_PORT 1883
#define MQTT_BROKER_USERNAME "unimaqtt"
#define MQTT_BROKER_PASSWORD "T5DRIIJEBgfhjsrFkaDERkgJhswMwk4"
#endif


/*Global Variables*/
extern bool mqtt_connected;
extern bool registered;
extern bool provisioned;
extern bool configured;
extern bool sending_ir_command;
extern bool teaching_in_progress;
extern char ir_protocol[20];
extern uint16_t publishPeriod; 