/**
 * @file queue.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to queue implementation
 * @version 0.2
 * @date 2024-03-30
 * @copyright Copyright (c) 2024
 */

#include "../../inc/Custom/queue.h"

void remove_from_pubmesg_queue()
{
	if(pubmesg_head_ptr->next == NULL)
	{
		pubmesg_head_ptr->next = NULL;
		pubmesg_head_ptr->prev = NULL;
		pubmesg_head_ptr = NULL;
		pubmesg_tail_ptr = NULL;
		return;
	}
	pubmesg_head_ptr = pubmesg_head_ptr->next;
	free(pubmesg_head_ptr->prev);
	pubmesg_head_ptr->prev = NULL;
}

/**
 * @brief Function that adds messages to the pubmesg queue. These will be published one by one to
 * cloud by a handler function. If successfully published, they will be removed from the queue.
 * @param msg The message to be published to cloud
 * @param topic The topic to which the message needs to be published
 * @warning This process is not threadsafe. Need to implement it as threadsafe.
 */
void add_to_pubmesg_queue(char *msg, char *topic)
{
	if(pubmesg_head_ptr!=NULL)
	{
		uint16_t msg_count = 1;
		struct pub_mesg_struct *ptr = pubmesg_head_ptr;
		ESP_LOGI(DEBUG_TAG, "Current Queue : ");
		while(ptr!=NULL)
		{
			ESP_LOGI(DEBUG_TAG, "\t%d) %s",msg_count, ptr->message);
			ptr = ptr->next;
			msg_count++;
		}
		printf("\n");
	}
	struct pub_mesg_struct *pubmesg_node = (struct pub_mesg_struct *)malloc(sizeof(struct pub_mesg_struct));
	if(pubmesg_node!=NULL)
	{
		//Adding very first element to queue
		if(pubmesg_head_ptr == NULL && pubmesg_tail_ptr == NULL)
		{
			printf("Adding the first element into queue ... \n");
			pubmesg_head_ptr = pubmesg_node;
			pubmesg_node->prev = NULL;
		}
		else
			pubmesg_node->prev = pubmesg_tail_ptr;
		pubmesg_node->next = NULL;
		pubmesg_tail_ptr = pubmesg_node;
		strcpy(pubmesg_node->message,msg);
		pubmesg_node->topic = topic;
	}
	else
		printf("Error in memory allocation while trying to add to queue ...\n");
}