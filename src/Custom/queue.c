/**
 * @file queue.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to queue implementation
 * @version 0.2
 * @date 2024-03-30
 * @copyright Copyright (c) 2024
 */

#include "../../inc/Custom/queue.h"

void search_node_pub_conf_queue(uint16_t messageNum)
{
	char pubmessage[PUBMESG_LEN];
	pub_conf_t *traverser = node_pub_conf_queue_head;
	while(traverser != NULL)
	{
		if(traverser->base_data.msg_seq_no == messageNum)
		{
			if(esp_timer_get_time() - traverser->base_data.request_in_time_us > NODE_TIMEOUT_INTERVAL_US) //10s
			{
				traverser->base_data.error_code = NODE_TIMEOUT;
				sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d}",
					JSON_PACKET_ID_KEY, NODE_TEMPERATURE_DATA_PACKET,
					JSON_ACK_NAME_KEY, NODE_TEMPERATURE_DATA_ACK,
					MSG_SEQ_NO_KEY, traverser->base_data.msg_seq_no,
					GWY_SER_NO_KEY, GWY_SER_NO,
					NODE_SER_NO_KEY, traverser->base_data.node_ser_no,
					ELMNT_ADDR_KEY, traverser->base_data.elementAddr,
					PUBLISH_PERIOD_KEY, traverser->pub_conf_period_in_sec,
					ERROR_CODE_KEY, traverser->base_data.error_code);
				add_to_pubmesg_queue(pubmessage, publish_topic);
			}
		}
		traverser = traverser->next;
	}
}

void remove_from_node_pub_conf_queue()
{
	if(node_pub_conf_queue_head == NULL)
	{
		sprintf(queue_log_buffer, "node_pub_conf_queue is empty\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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
	pub_conf_t *pub_conf_node = (pub_conf_t *)malloc(sizeof(pub_conf_t));
	if(pub_conf_node!=NULL) *pub_conf_node = node_pub_conf_t;
	else
	{
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_node_pub_conf_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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

/**
 * @brief Get the node_pubconf queue count
 * @param none
 * @return Number of elements currently in node_pubconf queue 
 */
uint8_t get_node_pub_conf_queue_count(pub_conf_t *head)
{
	uint8_t count = 0;
	while(head!=NULL)
	{
		count++;
		head=head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_node_pubconf_queue()
{
	pub_conf_t *temp = node_pub_conf_queue_head;
	while(temp!=NULL)
	{
		if(esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_TIMEOUT_INTERVAL_US)
		{
			temp=temp->next;
			remove_from_node_pub_conf_queue();
			continue;
		}
		temp=temp->next;
	}
}

void search_node_reconf_queue(uint16_t messageNum)
{
	char pubmessage[PUBMESG_LEN];
	reconf_t *traverser = node_reconf_queue_head;
	while(traverser != NULL)
	{
		if(traverser->base_data.msg_seq_no == messageNum)
		{
			if(esp_timer_get_time() - traverser->base_data.request_in_time_us > NODE_TIMEOUT_INTERVAL_US) //10s
			{
				traverser->base_data.error_code = NODE_TIMEOUT;
				sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d}",
					JSON_PACKET_ID_KEY, NODE_RECONF_PACKET,
					JSON_ACK_NAME_KEY, NODE_RECONF_ACK,
					MSG_SEQ_NO_KEY, traverser->base_data.msg_seq_no,
					GWY_SER_NO_KEY, GWY_SER_NO,
					NODE_SER_NO_KEY, traverser->base_data.elementAddr,
					ERROR_CODE_KEY, traverser->base_data.error_code);
				add_to_pubmesg_queue(pubmessage, publish_topic);
			}
		}
		traverser = traverser->next;
	}
}

void remove_from_node_reconf_queue()
{
	if(node_reconf_queue_head == NULL)
	{
		sprintf(queue_log_buffer, "node_reconf_queue is empty\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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
	reconf_t *reconf_node = (reconf_t *)malloc(sizeof(reconf_t));
	if(reconf_node!=NULL) *reconf_node = node_reconf_t;
	else
	{
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_node_reconf_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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

/**
 * @brief Get the node_reconf queue count
 * @param none
 * @return Number of elements currently in node_reconf queue 
 */
uint8_t get_node_reconf_queue_count(reconf_t *head)
{
	uint8_t count = 0;
	while(head!=NULL)
	{
		count++;
		head=head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_node_reconf_queue()
{
	reconf_t *temp = node_reconf_queue_head;
	while(temp!=NULL)
	{
		if(esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_TIMEOUT_INTERVAL_US)
		{
			temp=temp->next;
			remove_from_node_reconf_queue();
			continue;
		}
		temp=temp->next;
	}
}


void search_node_control_queue(uint16_t messageNum)
{
	char pubmessage[PUBMESG_LEN];
	control_t *traverser = node_ac_control_queue_head;
	while(traverser != NULL)
	{
		if(traverser->base_data.msg_seq_no == messageNum)
		{
			if(esp_timer_get_time() - traverser->base_data.request_in_time_us > NODE_TIMEOUT_INTERVAL_US) //10s
			{
				traverser->base_data.error_code = NODE_TIMEOUT;
				sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d}",
					JSON_PACKET_ID_KEY, NODE_AC_CONTROL_PACKET,
					JSON_ACK_NAME_KEY, NODE_AC_CONTROL_ACK,
					MSG_SEQ_NO_KEY, traverser->base_data.msg_seq_no,
					GWY_SER_NO_KEY, GWY_SER_NO,
					NODE_SER_NO_KEY, traverser->base_data.elementAddr,
					POWER_KEY, traverser->power,
					MODE_KEY, traverser->mode_str,
					FAN_SPEED_KEY, traverser->fan,
					TEMPERATURE_KEY, traverser->temp,
					SWING_H_KEY, traverser->swingH,
					SWING_V_KEY, traverser->swingV,
					ONTIMER_KEY, traverser->OnTimer,
					OFFTIMER_KEY, traverser->OffTimer,
					AC_LOCKING_KEY, traverser->Locking,
					TEMP_LOCK_UP_LIMIT_KEY, traverser->TempUpLimit,
					TEMP_LOCK_LOW_LIMIT_KEY, traverser->TempLowLimit,
					ERROR_CODE_KEY, traverser->base_data.error_code);
				add_to_pubmesg_queue(pubmessage, publish_topic);
			}
		}
		traverser = traverser->next;
	}
}

void remove_from_node_control_queue()
{
	if(node_ac_control_queue_head == NULL)
	{
		sprintf(queue_log_buffer, "node_ac_control_queue is empty\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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
	control_t *control_node = (control_t *)malloc(sizeof(control_t));
	if(control_node!=NULL) *control_node = node_ac_control_t;
	else
	{
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_unprov_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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

/**
 * @brief Get the node_ac_control queue count
 * @param none
 * @return Number of elements currently in node_ac_control queue 
 */
uint8_t get_node_control_queue_count(control_t *head)
{
	uint8_t count = 0;
	while(head!=NULL)
	{
		count++;
		head=head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_node_ac_control_queue()
{
	control_t *temp = node_ac_control_queue_head;
	while(temp!=NULL)
	{
		if(esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_TIMEOUT_INTERVAL_US)
		{
			temp=temp->next;
			remove_from_node_control_queue();
			continue;
		}
		temp=temp->next;
	}
}

void search_unprov_queue(uint16_t messageNum)
{
	char pubmessage[PUBMESG_LEN];
	unprov_t *traverser = unprov_queue_head;
	while(traverser != NULL)
	{
		if(traverser->base_data.msg_seq_no == messageNum)
		{
			if(esp_timer_get_time() - traverser->base_data.request_in_time_us > NODE_TIMEOUT_INTERVAL_US) //10s
			{
				traverser->base_data.error_code = NODE_TIMEOUT;
				sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %s, %s : %d}",
					JSON_PACKET_ID_KEY, NODE_UNPROV_PACKET,
					JSON_ACK_NAME_KEY, NODE_UNPROV_ACK,
					MSG_SEQ_NO_KEY, traverser->base_data.msg_seq_no,
					GWY_SER_NO_KEY, GWY_SER_NO,
					NODE_SER_NO_KEY, traverser->base_data.node_ser_no,
					ELMNT_ADDR_KEY, traverser->base_data.elementAddr,
					LOCATION_KEY, traverser->base_data.location,
					ERROR_CODE_KEY, traverser->base_data.error_code);
				add_to_pubmesg_queue(pubmessage, publish_topic);
			}
		}
		traverser = traverser->next;
	}
}

void remove_from_unprov_queue()
{
	if(unprov_queue_head == NULL)
	{
		sprintf(queue_log_buffer, "unprov_queue is empty\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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
	unprov_t *unprov_node = (unprov_t *)malloc(sizeof(unprov_t));
	if(unprov_node!=NULL) *unprov_node = unprovision_t;
	else
	{
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_unprov_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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

/**
 * @brief Get the unprov queue count
 * @param none
 * @return Number of elements currently in unprov queue 
 */
uint8_t get_unprov_queue_count(unprov_t *head)
{
	uint8_t count = 0;
	while(head!=NULL)
	{
		count++;
		head=head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_unprov_queue()
{
	unprov_t *temp = unprov_queue_head;
	while(temp!=NULL)
	{
		if(esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_TIMEOUT_INTERVAL_US)
		{
			temp=temp->next;
			remove_from_unprov_queue();
			continue;
		}
		temp=temp->next;
	}
}

void search_prov_queue(uint16_t messageNum)
{
	char pubmessage[PUBMESG_LEN];
	prov_t *traverser = prov_queue_head;
	while(traverser != NULL)
	{
		if(traverser->base_data.msg_seq_no == messageNum)
		{
			if(esp_timer_get_time() - traverser->base_data.request_in_time_us > NODE_TIMEOUT_INTERVAL_US) //10s
			{
				traverser->base_data.error_code = NODE_TIMEOUT;
				sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %s, %s : %d}",
					JSON_PACKET_ID_KEY, NODE_PROV_PACKET,
					JSON_ACK_NAME_KEY, NODE_PROV_ACK,
					MSG_SEQ_NO_KEY, traverser->base_data.msg_seq_no,
					GWY_SER_NO_KEY, GWY_SER_NO,
					NODE_SER_NO_KEY, traverser->base_data.node_ser_no,
					ELMNT_ADDR_KEY, traverser->base_data.elementAddr,
					LOCATION_KEY, traverser->base_data.location,
					ERROR_CODE_KEY, traverser->base_data.error_code);
				add_to_pubmesg_queue(pubmessage, publish_topic);
			}
		}
		traverser = traverser->next;
	}
}

void remove_from_prov_queue()
{
	if(prov_queue_head == NULL)
	{
		sprintf(queue_log_buffer, "prov_queue is empty\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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
	prov_t *prov_node = (prov_t *)malloc(sizeof(prov_t));
	if(prov_node!=NULL) *prov_node = provision_t;
	else 
	{
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_prov_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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

/**
 * @brief Get the prov queue count
 * @param none
 * @return Number of elements currently in prov queue 
 */
uint8_t get_prov_queue_count(prov_t *head)
{
	uint8_t count = 0;
	while(head!=NULL)
	{
		count++;
		head=head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_prov_queue()
{
	prov_t *temp = prov_queue_head;
	while(temp!=NULL)
	{
		if(esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_TIMEOUT_INTERVAL_US)
		{
			temp=temp->next;
			remove_from_prov_queue();
			continue;
		}
		temp=temp->next;
	}
}

void remove_from_pubmesg_queue()
{
	if(pubmesg_queue_head == NULL)
	{
		sprintf(queue_log_buffer, "pubmesg queue is empty\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
		return;
	}
	else
	{
		struct pub_mesg_struct *temp = pubmesg_queue_head;
		pubmesg_queue_head = pubmesg_queue_head->next;
		free(temp);
		if(pubmesg_queue_head == NULL) pubmesg_queue_tail = NULL;
	}
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
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_pubmesg_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
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
 * @brief Get the pubmesg queue count
 * @param none
 * @return Number of elements currently in Pubmesg queue 
 */
uint8_t get_pubmesg_queue_count(pubmesg_t *head)
{
	uint8_t count = 0;
	while(head!=NULL)
	{
		count++;
		head=head->next;
	}
	return count;
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
void queue_handler(void *args)
{
	while(1)
	{
		vTaskDelay(1);
		if(!sending)
		{
			if(prov_queue_head != NULL)
			{
				snprintf(queue_log_buffer, sizeof(queue_log_buffer), "prov_queue_count(%d)",get_prov_queue_count(prov_queue_head));
				yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
				maintain_prov_queue();
				send_prov_packet_to_node(prov_queue_head);
			}
			if(unprov_queue_head != NULL)
			{
				snprintf(queue_log_buffer, sizeof(queue_log_buffer), "node_unprov_queue_count(%d)",get_unprov_queue_count(unprov_queue_head));
				yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
				maintain_unprov_queue();
				send_unprov_packet_to_node(unprov_queue_head);
			}
			if(node_reconf_queue_head != NULL)
			{
				snprintf(queue_log_buffer, sizeof(queue_log_buffer), "node_reconf_queue_count(%d)",get_node_reconf_queue_count(node_reconf_queue_head));
				yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
				maintain_node_reconf_queue();
				send_reconf_packet_to_node(node_reconf_queue_head);
			}	
			if(node_ac_control_queue_head != NULL)
			{
				snprintf(queue_log_buffer, sizeof(queue_log_buffer), "node_ac_control_queue_count(%d)",get_node_control_queue_count(node_ac_control_queue_head));
				yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
				maintain_node_ac_control_queue();
				send_ac_control_packet_to_node(node_ac_control_queue_head);
			}
			if(node_pub_conf_queue_head != NULL)
			{
				snprintf(queue_log_buffer, sizeof(queue_log_buffer), "node_pubconf_queue_count(%d)",get_node_pub_conf_queue_count(node_pub_conf_queue_head));
				yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
				maintain_node_pubconf_queue();
				send_pub_conf_packet_to_node(node_pub_conf_queue_head);
			}
			//Don't try to publish in the middle of sending an IR command
			if(pubmesg_queue_head != NULL && mqtt_connected)
			{
				snprintf(queue_log_buffer, sizeof(queue_log_buffer), "BEFORE: pubmesg_queue_count(%d)",get_pubmesg_queue_count(pubmesg_queue_head));
				yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
				if(publish_to_mqtt() == SUCCESS)
				{
					remove_from_pubmesg_queue();
					snprintf(queue_log_buffer, sizeof(queue_log_buffer), "AFTER: pubmesg_queue_count(%d)",get_pubmesg_queue_count(pubmesg_queue_head));
					yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
				}
			}
		}
	}
}