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
    //Pubmesg
    int8_t publish_to_mqtt();
    void remove_from_pubmesg_queue();   
	void add_to_pubmesg_queue(char *msg, char *topic);
    uint8_t get_pubmesg_queue_count(pubmesg_t *);
    
    //Prov
    void remove_from_prov_queue();
    void add_to_prov_queue();
    uint8_t get_prov_queue_count(prov_t *);
    void maintain_prov_queue();

    //Unprov
    void remove_from_unprov_queue();
    void add_to_unprov_queue();
    uint8_t get_unprov_queue_count(unprov_t *);
    void maintain_unprov_queue();

    //Node AC control
    void remove_from_node_control_queue();
    void add_to_node_control_queue();
    uint8_t get_node_control_queue_count(control_t *);
    void maintain_node_ac_control_queue();

    //Node Reconf
    void remove_from_node_reconf_queue();
    void add_to_node_reconf_queue();
    uint8_t get_node_reconf_queue_count(reconf_t *);
    void maintain_node_reconf_queue();

    //Node Pubconf
    void remove_from_node_pub_conf_queue();
    void add_to_node_pub_conf_queue();
    uint8_t get_node_pub_conf_queue_count(pub_conf_t *);
    void maintain_node_pubconf_queue();

    //Node Teaching Mode
    void remove_from_node_teaching_mode_queue();
    void add_to_node_teaching_mode_queue();
    uint8_t get_node_teaching_mode_queue_count();
    void maintain_node_teaching_mode_queue();

    //Node Debug info
    void remove_from_node_debug_info_queue();
    void add_to_node_debug_info_queue();
    uint8_t get_node_debug_info_queue_count();
    void maintain_node_debug_info_queue();
    
    void queue_handler(void *args);
#ifdef __cplusplus
}
#endif