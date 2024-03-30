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
#ifdef __cplusplus
}
#endif