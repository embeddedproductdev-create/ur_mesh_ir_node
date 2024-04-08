/**
 * @file queue.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all definitions and functions related to queue implementation
 * @version 0.2
 * @date 2024-03-30
 * @copyright Copyright (c) 2024
 */

#include "main.h"

/* GLOBAL VARIABLES */

/* FUNCTION DECLARATIONS */
#ifdef __cplusplus
extern "C"
{
#endif
    int8_t publish_to_mqtt();
    void remove_from_pubmesg_queue();   
	void add_to_pubmesg_queue(char *msg, char *topic);
    void remove_from_prov_queue();
    void add_to_prov_queue();
    void remove_from_unprov_queue();
    void add_to_unprov_queue();
    void remove_from_node_control_queue();
    void add_to_node_control_queue();
    void remove_from_node_reconf_queue();
    void add_to_node_reconf_queue();
    void remove_from_node_pub_conf_queue();
    void add_to_node_pub_conf_queue();
    void *queue_handler(void *args);
#ifdef __cplusplus
}
#endif