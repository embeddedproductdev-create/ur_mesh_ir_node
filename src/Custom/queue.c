/**
 * @file queue.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to queue implementation
 * @version 0.2
 * @date 2024-03-30
 * @copyright Copyright (c) 2024
 */

#include "../../inc/Custom/queue.h"

void remove_from_node_pub_conf_queue()
{
	if(node_pub_conf_queue_head == NULL)
	{
		ESP_LOGE(ERROR_TAG, "node_pub_conf_queue is empty\r\n");
		return;
	}
	else
	{
		pub_conf_t *temp = node_pub_conf_queue_head;
		node_reconf_queue_head = node_reconf_queue_head->next;
		free(temp);
		if(node_reconf_queue_head == NULL) node_reconf_queue_tail = NULL;
	}
}

void add_to_node_pub_conf_queue()
{
	pub_conf_t *pub_conf_node = (struct pub_conf_t *)malloc(sizeof(pub_conf_t));
	if(pub_conf_node!=NULL) *pub_conf_node = node_pub_conf_t;
	else
	{
		ESP_LOGE(ERROR_TAG, "Memory allocation failed in add_to_node_pub_conf_queue\r\n");
		return;
	}
	if (node_pub_conf_queue_head == NULL)
	{
		//Adding the first element into the queue
		node_pub_conf_queue_head = node_pub_conf_queue_tail = pub_conf_node;
		pub_conf_node->next = pub_conf_node->prev = NULL;
	}
	else
	{
		node_pub_conf_queue_tail->next = pub_conf_node;
		node_pub_conf_queue_tail->next->prev = node_pub_conf_queue_tail;
		node_pub_conf_queue_tail->next->next = NULL;
		node_pub_conf_queue_tail = pub_conf_node;
	}
}

void remove_from_node_reconf_queue()
{
	if(node_reconf_queue_head == NULL)
	{
		ESP_LOGE(ERROR_TAG, "node_reconf_queue is empty\r\n");
		return;
	}
	else
	{
		reconf_t *temp = node_reconf_queue_head;
		node_reconf_queue_head = node_reconf_queue_head->next;
		free(temp);
		if(node_reconf_queue_head == NULL) node_reconf_queue_tail = NULL;
	}
}

void add_to_node_reconf_queue()
{
	reconf_t *reconf_node = (struct reconf_t *)malloc(sizeof(reconf_t));
	if(reconf_node!=NULL) *reconf_node = node_reconf_t;
	else
	{
		ESP_LOGE(ERROR_TAG, "Memory allocation failed in add_to_node_reconf_queue\r\n");
		return;
	}
	if (node_reconf_queue_head == NULL)
	{
		//Adding the first element into the queue
		node_reconf_queue_head = node_reconf_queue_tail = reconf_node;
		reconf_node->next = reconf_node->prev = NULL;
	}
	else
	{
		node_reconf_queue_tail->next = reconf_node;
		node_reconf_queue_tail->next->prev = node_reconf_queue_tail;
		node_reconf_queue_tail->next->next = NULL;
		node_reconf_queue_tail = reconf_node;
	}
}

void remove_from_node_control_queue()
{
	if(node_ac_control_queue_head == NULL)
	{
		ESP_LOGE(ERROR_TAG, "node_ac_control_queue is empty\r\n");
		return;
	}
	else
	{
		control_t *temp = node_ac_control_queue_head;
		node_ac_control_queue_head = node_ac_control_queue_head->next;
		free(temp);
		if(node_ac_control_queue_head == NULL) node_ac_control_queue_tail = NULL;
	}
}

void add_to_node_control_queue()
{
	control_t *control_node = (struct control_t *)malloc(sizeof(control_t));
	if(control_node!=NULL) *control_node = node_ac_control_t;
	else
	{
		ESP_LOGE(ERROR_TAG, "Memory allocation failed in add_to_unprov_queue\r\n");
		return;
	}
	if (node_ac_control_queue_head == NULL)
	{
		//Adding the first element into the queue
		node_ac_control_queue_head = node_ac_control_queue_tail = control_node;
		control_node->next = control_node->prev = NULL;
	}
	else
	{
		node_ac_control_queue_tail->next = control_node;
		node_ac_control_queue_tail->next->prev = node_ac_control_queue_tail;
		node_ac_control_queue_tail->next->next = NULL;
		node_ac_control_queue_tail = control_node;
	}
}

void remove_from_unprov_queue()
{
	if(unprov_queue_head == NULL)
	{
		ESP_LOGE(ERROR_TAG, "unprov_queue is empty\r\n");
		return;
	}
	else
	{
		unprov_t *temp = unprov_queue_head;
		unprov_queue_head = unprov_queue_head->next;
		free(temp);
		if(unprov_queue_head == NULL) unprov_queue_tail = NULL;
	}
}

void add_to_unprov_queue()
{
	unprov_t *unprov_node = (struct unprov_t *)malloc(sizeof(unprov_t));
	if(unprov_node!=NULL) *unprov_node = unprovision_t;
	else
	{
		ESP_LOGE(ERROR_TAG, "Memory allocation failed in add_to_unprov_queue\r\n");
		return;
	}
	if (unprov_queue_head == NULL)
	{
		//Adding the first element into the queue
		unprov_queue_head = unprov_queue_tail = unprov_node;
		unprov_node->next = unprov_node->prev = NULL;
	}
	else
	{
		unprov_queue_tail->next = unprov_node;
		unprov_queue_tail->next->prev = unprov_queue_tail;
		unprov_queue_tail->next->next = NULL;
		unprov_queue_tail = unprov_node;
	}
}

void remove_from_prov_queue()
{
	if(prov_queue_head == NULL)
	{
		ESP_LOGE(ERROR_TAG, "prov_queue is empty\r\n");
		return;
	}
	else
	{
		prov_t *temp = prov_queue_head;
		prov_queue_head = prov_queue_head->next;
		free(temp);
		if(prov_queue_head == NULL) prov_queue_tail = NULL;
	}
}

void add_to_prov_queue()
{
	prov_t *prov_node = (struct prov_t *)malloc(sizeof(prov_t));
	if(prov_node!=NULL) *prov_node = provision_t;
	else 
	{
		ESP_LOGE(ERROR_TAG, "Memory allocation failed in add_to_prov_queue\r\n");
		return;
	}
	if (prov_queue_head == NULL)
	{
		//Adding the first element into the queue
		prov_queue_head = prov_queue_tail = prov_node;
		prov_node->next = prov_node->prev = NULL;
	}
	else
	{
		prov_queue_tail->next = prov_node;
		prov_queue_tail->next->prev = prov_queue_tail;
		prov_queue_tail->next->next = NULL;
		prov_queue_tail = prov_node;
	}
}

void remove_from_pubmesg_queue()
{
	if(pubmesg_queue_head->next == NULL)
	{
		pubmesg_queue_head->next = NULL;
		pubmesg_queue_head->prev = NULL;
		pubmesg_queue_head = NULL;
		pubmesg_queue_tail = NULL;
		return;
	}
	pubmesg_queue_head = pubmesg_queue_head->next;
	free(pubmesg_queue_head->prev);
	pubmesg_queue_head->prev = NULL;
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
	struct pub_mesg_struct *pubmesg_node = (struct pub_mesg_struct *)malloc(sizeof(struct pub_mesg_struct));
	if(pubmesg_node == NULL) 
	{
		ESP_LOGE(ERROR_TAG, "Memory allocation failed in add_to_pubmesg_queue\r\n");
		return;
	}
	strcpy(pubmesg_node->message,msg);
	pubmesg_node->topic = topic;
	if(pubmesg_queue_head == NULL)
	{
		pubmesg_queue_head = pubmesg_queue_tail = pubmesg_node;
		pubmesg_node->prev = pubmesg_node->next = NULL;
	}
	else
	{
		pubmesg_queue_tail->next = pubmesg_node;
		pubmesg_queue_tail->next->prev = pubmesg_queue_tail;
		pubmesg_queue_tail->next->next = NULL;
		pubmesg_queue_tail = pubmesg_node;
	}
}

/**
 * @brief Thread that takes care of handling all queues throught out the code 
 * We have the following queues across the application. We can call it a queue or LinkedLists.
 * They are actually Doubly linked lists with structures as node. 
 *  - Provision List
 *  - Unprovision List
 *  - Node Reconfiguration List
 *  - Node AC control List
 *  - Node Publish configuration List
 *  - Publish Message List
 * @param args 
 * @return void* 
 */
void *queue_handler(void *args)
{
	while(1)
	{
		vTaskDelay(1);
		if(prov_queue_head != NULL)
		{
			send_prov_packet_to_node(prov_queue_head);
		}
		if(unprov_queue_head != NULL)
		{
			send_unprov_packet_to_node(unprov_queue_head);
		}
		if(node_reconf_queue_head != NULL)
		{
			send_reconf_packet_to_node(node_reconf_queue_head);
		}	
		if(node_ac_control_queue_head != NULL)
		{
			send_ac_control_packet_to_node(node_ac_control_queue_head);
		}
		if(node_pub_conf_queue_head != NULL)
		{
			send_pub_conf_packet_to_node(node_pub_conf_queue_head);
		}
		if(pubmesg_queue_head != NULL)
		{
			
		}
	}
}