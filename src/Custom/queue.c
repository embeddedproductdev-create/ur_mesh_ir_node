/**
 * @file queue.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to queue implementation
 * @version 0.2
 * @date 2024-03-30
 * @copyright Copyright (c) 2024
 */

#include "../../inc/Custom/queue.h"

//Initialization
char pubmessage[PUBMESG_LEN];

void search_node_pub_conf_queue(uint16_t messageNum)
{
	char pubmessage[PUBMESG_LEN];
	pub_conf_t *traverser = node_pub_conf_queue_head;
	while(traverser != NULL)
	{
		if(traverser->base_data.msg_seq_no == messageNum)
		{
			//write some logic here
			;
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
		node_pub_conf_queue_head = node_pub_conf_queue_head->next;
		free(temp);
		if(node_pub_conf_queue_head == NULL) node_pub_conf_queue_tail = NULL;
	}
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Node pubconf queue | Node Pubconf Queue Count(%d)", get_node_pub_conf_queue_count(node_pub_conf_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
	if(LOG_DATA)
	{
		snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Node pubconf queue | Node Pubconf Queue Count(%d)", get_node_pub_conf_queue_count(node_pub_conf_queue_head));
		yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
			sprintf(queue_log_buffer, "Removing NodePubConf request(msgseqno : %d) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %s, %s : %d, %s : %d, %s : %d",
			JSON_PACKET_ID_KEY, NODE_PUB_CONF_PACKET,
			JSON_ACK_NAME_KEY, NODE_PUB_CONF_ACK,
			MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
			GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
			NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
			ELMNT_ADDR_KEY, temp->base_data.elementAddr,
			PUBLISH_PERIOD_KEY, temp->pub_conf_period_in_sec,
			ERROR_CODE_KEY, NODE_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_node_pub_conf_queue();
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
				//Write some logic here
				;
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
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Node reconf Queue | Node Reconf Queue Count(%d)", get_node_reconf_queue_count(node_reconf_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
	if(LOG_DATA)
	{
		snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Node reconf Queue | Node Reconf Queue Count(%d)", get_node_reconf_queue_count(node_reconf_queue_head));
		yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
			sprintf(queue_log_buffer, "Removing NodeReconf request(msgseqno : %d) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %s, %s : %d, %s : %d",
			JSON_PACKET_ID_KEY, NODE_RECONF_PACKET,
			JSON_ACK_NAME_KEY, NODE_RECONF_ACK,
			MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
			GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
			NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
			ELMNT_ADDR_KEY, temp->base_data.elementAddr,
			ERROR_CODE_KEY, NODE_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_node_reconf_queue();
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
			//Write some logic here
			;
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
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Node AC control Queue | Node AC control Queue Count(%d)", get_node_control_queue_count(node_ac_control_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
	if(LOG_DATA)
	{
		snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Node AC control Queue | Node AC control Queue Count(%d)", get_node_control_queue_count(node_ac_control_queue_head));
		yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
			sprintf(queue_log_buffer, "Removing NodeACControl request(msgseqno : %d) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer); 
			sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d,, %s : %s, %s : %s, %s : %d, %s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d}",
                JSON_PACKET_ID_KEY, NODE_AC_CONTROL_PACKET,
                JSON_ACK_NAME_KEY, NODE_AC_CONTROL_ACK,
				MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
                GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
				NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
				ELMNT_ADDR_KEY, temp->base_data.elementAddr,
                POWER_KEY, temp->power,
                MODE_KEY, temp->mode_str,
                FAN_SPEED_KEY, temp->fan,
                TEMPERATURE_KEY, temp->temp,
                SWING_H_KEY, temp->swingH,
                SWING_V_KEY, temp->swingV,
                ONTIMER_KEY, temp->OnTimer,
                OFFTIMER_KEY, temp->OffTimer,
				AC_LOCKING_KEY, temp->Locking,
				TEMP_LOCK_UP_LIMIT_KEY, temp->TempUpLimit,
				TEMP_LOCK_LOW_LIMIT_KEY, temp->TempLowLimit,
                ERROR_CODE_KEY, NODE_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_node_control_queue();
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
				//Write some logic here
				;
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
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Unprov Queue | Unprov Queue count(%d)", get_unprov_queue_count(unprov_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
	if(LOG_DATA)
	{
		snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Unprov Queue | Unprov Queue count(%d)", get_unprov_queue_count(unprov_queue_head));
		yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
			sprintf(queue_log_buffer, "Removing NodeUnprov request(msgseqno : %d) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %s, %s : %d, %s : %d",
				JSON_PACKET_ID_KEY, NODE_UNPROV_PACKET,
				JSON_ACK_NAME_KEY, NODE_UNPROV_ACK,
				MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
				GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
				NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
				ELMNT_ADDR_KEY, temp->base_data.elementAddr,
				ERROR_CODE_KEY, NODE_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_unprov_queue();
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
				//Write some logic here
				;
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
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Prov Queue | Prov Queue Count(%d)", get_prov_queue_count(prov_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
	if(LOG_DATA)
	{
		snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Prov Queue | Prov Queue Count(%d)", get_prov_queue_count(prov_queue_head));
		yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
			sprintf(queue_log_buffer, "Removing Prov request(msgseqno : %d) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %s, %s : %d}",
					JSON_PACKET_ID_KEY, NODE_PROV_PACKET,
					JSON_ACK_NAME_KEY, NODE_PROV_ACK,
					MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
					GWY_SER_NO_KEY, GWY_SER_NO,
					NODE_SER_NO_KEY, temp->base_data.node_ser_no,
					ELMNT_ADDR_KEY, temp->base_data.elementAddr,
					LOCATION_KEY, temp->base_data.location,
					ERROR_CODE_KEY, NODE_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_prov_queue();
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
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Pubmesg Queue | Pubmesg Queue Count(%d)", get_pubmesg_queue_count(pubmesg_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
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
	//If it's bad time to add to pubmesg due to LTE no response, then hold off and don't keep adding. This will lead to OOM eventually.
	if(!hold_adding_to_pubmesg)
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
		if(LOG_DATA)
		{
			snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Pubmesg Queue | Pubmesg Queue Count(%d)", get_pubmesg_queue_count(pubmesg_queue_head));
			yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
		}
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
 * They are actually Doubly linked lists with structures as nodes. 
 *  - Provision List - Conaints list of prov requests from MQTT
 *  - Unprovision List - Contains list of Unprov requests from MQTT
 *  - Node Reconfiguration List - Contains list of reconf requests from MQTT
 *  - Node AC control List - Contains list of AC control requests from MQTT
 *  - Node Publish configuration List - Contains list of Pubconf requests from MQTT
 *  - Publish Message List - Contains lists of messages to be published to MQTT
 * @param args 
 * @return void* 
 */
void queue_handler(void *args)
{
	while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(100));
		if(!needToSendIRComamnd)
		{
			if(prov_queue_head != NULL)
			{
				maintain_prov_queue();
				//Don't keep sending the requests again and again
				if(prov_queue_head != NULL)
				{
					if(!prov_queue_head->base_data.request_sent_to_node_flag) {
						prov_queue_head->base_data.request_sent_to_node_flag = true;
						send_prov_packet_to_node(prov_queue_head);
					}
				}
			}
			if(unprov_queue_head != NULL)
			{
				maintain_unprov_queue();
				//Don't keep sending the requests again and again
				if(unprov_queue_head != NULL)
				{
					if(!unprov_queue_head->base_data.request_sent_to_node_flag) {
						unprov_queue_head->base_data.request_sent_to_node_flag = true;
						send_unprov_packet_to_node(unprov_queue_head);
					}
				}
			}
			if(node_reconf_queue_head != NULL)
			{
				maintain_node_reconf_queue();
				//Don't keep sending the requests again and again
				if(node_reconf_queue_head != NULL)
				{
					if(!node_reconf_queue_head->base_data.request_sent_to_node_flag) {
						node_reconf_queue_head->base_data.request_sent_to_node_flag = true;
						send_reconf_packet_to_node(node_reconf_queue_head);
					}
				}
			}	
			if(node_ac_control_queue_head != NULL)
			{
				maintain_node_ac_control_queue();
				//Don't keep sending the requests again and again
				if(node_ac_control_queue_head != NULL)
				{
					if(!node_ac_control_queue_head->base_data.request_sent_to_node_flag) {
						node_ac_control_queue_head->base_data.request_sent_to_node_flag = true;
						send_ac_control_packet_to_node(node_ac_control_queue_head);
					}
				}
			}
			if(node_pub_conf_queue_head != NULL)
			{
				maintain_node_pubconf_queue();
				//Don't keep sending the requests again and again
				if(node_pub_conf_queue_head != NULL)
				{
					if(!node_pub_conf_queue_head->base_data.request_sent_to_node_flag) {
						node_pub_conf_queue_head->base_data.request_sent_to_node_flag = true;
						send_pub_conf_packet_to_node(node_pub_conf_queue_head);
					}
				}	
			}
			// //Don't try to publish in the middle of sending an IR command or while sending another AT command
			// //Some form of synchronization is required here.
			// if(pubmesg_queue_head != NULL && mqtt_connected && !sending_at_cmd)
			// {
			// 	if(publish_to_mqtt() == SUCCESS){ 
			// 		remove_from_pubmesg_queue();
			// 		if(LOG_DATA)
			// 		{
			// 			snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Successfully published and removed from Queue");
			// 			yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
			// 		}
			// 	}
			// 	else {
			// 		snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Failed to publish to MQTT");
			// 		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			// 	}
			// }
		}
	}
}