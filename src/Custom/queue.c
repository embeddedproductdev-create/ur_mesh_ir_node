/**
 * @file queue.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to queue implementation
 * @version 0.2
 * @date 2024-03-30
 * @copyright Copyright (c) 2024
 */

#include "../../inc/Custom/queue.h"

// Initialization
char pubmessage[PUBMESG_LEN];

/*-----------------------------------------------------------------------------------------*/
/**
 * @brief Function that removes elements from Node Publish Configuration Queue
 * @param none
 * @retval none
 */
void remove_from_heartbeat_pub_conf_queue()
{
	if (node_pub_conf_queue_head == NULL)
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
		if (node_pub_conf_queue_head == NULL)
			node_pub_conf_queue_tail = NULL;
	}
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Node pubconf queue | Node Pubconf Queue Count(%d)", get_heartbeat_pub_conf_queue_count(node_pub_conf_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

/**
 * @brief Function that adds elements to Node Publish Configuration queue
 * @param none
 * @retval none
 */
void add_to_heartbeat_pub_conf_queue()
{
	pub_conf_t *pub_conf_node = (pub_conf_t *)malloc(sizeof(pub_conf_t));
	if (pub_conf_node != NULL)
		*pub_conf_node = node_heartbeat_pub_conf_t;
	else
	{
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_heartbeat_pub_conf_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
		return;
	}
	if (node_pub_conf_queue_head == NULL)
	{
		// Adding the first element into the queue
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
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Node pubconf queue | Node Pubconf Queue Count(%d)", get_heartbeat_pub_conf_queue_count(node_pub_conf_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

/**
 * @brief Get the node_pubconf queue count
 * @param none
 * @return Number of elements currently in node_pubconf queue
 */
uint8_t get_heartbeat_pub_conf_queue_count(pub_conf_t *head)
{
	uint8_t count = 0;
	while (head != NULL)
	{
		count++;
		head = head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_COMM_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_heartbeat_pubconf_queue()
{
	pub_conf_t *temp = node_pub_conf_queue_head;
	while (temp != NULL)
	{
		if (esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_COMM_TIMEOUT_INTERVAL_US)
		{
			sprintf(queue_log_buffer, "Removing NodePubConf request(msgseqno : %ld) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d, \"%s\" : %d}",
					JSON_PACKET_ID_KEY, NODE_HEARTBEAT_PUB_CONF_PACKET,
					JSON_ACK_NAME_KEY, NODE_HEARTBEAT_PUB_CONF_ACK_NAME,
					MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
					GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
					NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
					ELEMENT_ADDR_KEY, temp->base_data.elementAddr,
					PUBLISH_PERIOD_KEY, temp->pub_conf_period_in_sec,
					ERROR_CODE_KEY, NODE_COMM_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_heartbeat_pub_conf_queue();
		}
		temp = temp->next;
	}
}

/*-----------------------------------------------------------------------------------------*/

void remove_from_reconf_queue()
{
	if (node_reconf_queue_head == NULL)
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
		if (node_reconf_queue_head == NULL)
			node_reconf_queue_tail = NULL;
	}
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Node reconf Queue | Node Reconf Queue Count(%d)", get_reconf_queue_count(node_reconf_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

void add_to_reconf_queue()
{
	reconf_t *reconf_node = (reconf_t *)malloc(sizeof(reconf_t));
	if (reconf_node != NULL)
		*reconf_node = node_reconf_t;
	else
	{
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_reconf_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
		return;
	}
	if (node_reconf_queue_head == NULL)
	{
		// Adding the first element into the queue
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
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Node reconf Queue | Node Reconf Queue Count(%d)", get_reconf_queue_count(node_reconf_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

/**
 * @brief Get the node_reconf queue count
 * @param none
 * @return Number of elements currently in node_reconf queue
 */
uint8_t get_reconf_queue_count(reconf_t *head)
{
	uint8_t count = 0;
	while (head != NULL)
	{
		count++;
		head = head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_COMM_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_reconf_queue()
{
	reconf_t *temp = node_reconf_queue_head;
	while (temp != NULL)
	{
		if (esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_COMM_TIMEOUT_INTERVAL_US)
		{
			sprintf(queue_log_buffer, "Removing NodeReconf request(msgseqno : %ld) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d}",
					JSON_PACKET_ID_KEY, NODE_RECONF_PACKET,
					JSON_ACK_NAME_KEY, NODE_RECONF_ACK_NAME,
					MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
					GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
					NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
					ELEMENT_ADDR_KEY, temp->base_data.elementAddr,
					ERROR_CODE_KEY, NODE_COMM_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_reconf_queue();
		}
		temp = temp->next;
	}
}

/*-----------------------------------------------------------------------------------------*/

void remove_from_ac_control_queue()
{
	if (node_ac_control_queue_head == NULL)
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
		if (node_ac_control_queue_head == NULL)
			node_ac_control_queue_tail = NULL;
	}
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Node AC control Queue | Node AC control Queue Count(%d)", get_ac_control_queue_count(node_ac_control_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

void add_to_ac_control_queue()
{
	control_t *control_node = (control_t *)malloc(sizeof(control_t));
	if (control_node != NULL)
		*control_node = node_ac_control_t;
	else
	{
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_unprov_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
		return;
	}
	if (node_ac_control_queue_head == NULL)
	{
		// Adding the first element into the queue
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
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Node AC control Queue | Node AC control Queue Count(%d)", get_ac_control_queue_count(node_ac_control_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

/**
 * @brief Get the node_ac_control queue count
 * @param none
 * @return Number of elements currently in node_ac_control queue
 */
uint8_t get_ac_control_queue_count(control_t *head)
{
	uint8_t count = 0;
	while (head != NULL)
	{
		count++;
		head = head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_COMM_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_ac_control_queue()
{
	control_t *temp = node_ac_control_queue_head;
	while (temp != NULL)
	{
		if (esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_COMM_TIMEOUT_INTERVAL_US)
		{
			sprintf(queue_log_buffer, "Removing NodeACControl request(msgseqno : %ld) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld,, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d}",
					JSON_PACKET_ID_KEY, NODE_AC_CONTROL_PACKET,
					JSON_ACK_NAME_KEY, NODE_AC_CONTROL_ACK_NAME,
					MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
					GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
					NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
					ELEMENT_ADDR_KEY, temp->base_data.elementAddr,
					POWER_KEY, temp->control.power,
					MODE_KEY, temp->control.mode_str,
					FAN_SPEED_KEY, temp->control.fanSpeed,
					TEMPERATURE_KEY, temp->control.temp,
					SWING_H_KEY, temp->control.swingH,
					SWING_V_KEY, temp->control.swingV,
					ONTIMER_KEY, temp->control.OnTimer,
					OFFTIMER_KEY, temp->control.OffTimer,
					AC_LOCKING_KEY, temp->control.Locking,
					TEMP_LOCK_UP_LIMIT_KEY, temp->control.TempLockUpLimit,
					TEMP_LOCK_LOW_LIMIT_KEY, temp->control.TempLockLowLimit,
					ERROR_CODE_KEY, NODE_COMM_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_ac_control_queue();
		}
		temp = temp->next;
	}
}

/*-----------------------------------------------------------------------------------------*/

void remove_from_unprov_queue()
{
	if (unprov_queue_head == NULL)
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
		if (unprov_queue_head == NULL)
			unprov_queue_tail = NULL;
	}
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Unprov Queue | Unprov Queue count(%d)", get_unprov_queue_count(unprov_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

void add_to_unprov_queue()
{
	unprov_t *unprov_node = (unprov_t *)malloc(sizeof(unprov_t));
	if (unprov_node != NULL)
		*unprov_node = unprovision_t;
	else
	{
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_unprov_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
		return;
	}
	if (unprov_queue_head == NULL)
	{
		// Adding the first element into the queue
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
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Unprov Queue | Unprov Queue count(%d)", get_unprov_queue_count(unprov_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

/**
 * @brief Get the unprov queue count
 * @param none
 * @return Number of elements currently in unprov queue
 */
uint8_t get_unprov_queue_count(unprov_t *head)
{
	uint8_t count = 0;
	while (head != NULL)
	{
		count++;
		head = head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_COMM_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_unprov_queue()
{
	unprov_t *temp = unprov_queue_head;
	while (temp != NULL)
	{
		if (esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_COMM_TIMEOUT_INTERVAL_US)
		{
			sprintf(queue_log_buffer, "Removing NodeUnprov request(msgseqno : %ld) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d}",
					JSON_PACKET_ID_KEY, NODE_UNPROV_PACKET,
					JSON_ACK_NAME_KEY, NODE_UNPROV_ACK_NAME,
					MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
					GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
					NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
					ELEMENT_ADDR_KEY, temp->base_data.elementAddr,
					LOCATION_KEY, temp->base_data.location,
					ERROR_CODE_KEY, NODE_COMM_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_unprov_queue();
		}
		temp = temp->next;
	}
}

/*-----------------------------------------------------------------------------------------*/

void remove_from_prov_queue()
{
	if (prov_queue_head == NULL)
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
		if (prov_queue_head == NULL)
			prov_queue_tail = NULL;
	}
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Prov Queue | Prov Queue Count(%d)", get_prov_queue_count(prov_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

void add_to_prov_queue()
{
	prov_t *prov_node = (prov_t *)malloc(sizeof(prov_t));
	if (prov_node != NULL)
		*prov_node = provision_t;
	else
	{
		sprintf(queue_log_buffer, "Memory allocation failed in add_to_prov_queue\r\n");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
		return;
	}
	if (prov_queue_head == NULL)
	{
		// Adding the first element into the queue
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
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Prov Queue | Prov Queue Count(%d)", get_prov_queue_count(prov_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

/**
 * @brief Get the prov queue count
 * @param none
 * @return Number of elements currently in prov queue
 */
uint8_t get_prov_queue_count(prov_t *head)
{
	uint8_t count = 0;
	while (head != NULL)
	{
		count++;
		head = head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_COMM_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_prov_queue()
{
	prov_t *temp = prov_queue_head;
	while (temp != NULL)
	{
		if (esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_COMM_TIMEOUT_INTERVAL_US)
		{
			sprintf(queue_log_buffer, "Removing Prov request(msgseqno : %ld) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d}",
					JSON_PACKET_ID_KEY, NODE_PROV_PACKET,
					JSON_ACK_NAME_KEY, NODE_PROV_ACK_NAME,
					MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
					GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
					NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
					ELEMENT_ADDR_KEY, temp->base_data.elementAddr,
					LOCATION_KEY, temp->base_data.location,
					ERROR_CODE_KEY, NODE_COMM_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_prov_queue();
		}
		temp = temp->next;
	}
}

/*-----------------------------------------------------------------------------------------*/

/**
 * @brief Function that removes elements from node teaching mode queue
 * @param none
 * @retval none
 */
void remove_from_teaching_mode_queue()
{
	if (node_teaching_mode_queue_head == NULL)
	{
		red_printf(QUEUE_ERROR_TAG, "node teaching mode queue is empty");
		return;
	}
	else
	{
		teaching_mode_t *temp = node_teaching_mode_queue_head;
		node_teaching_mode_queue_head = node_teaching_mode_queue_head->next;
		free(temp);
		if (node_teaching_mode_queue_head == NULL)
			node_teaching_mode_queue_tail = NULL;
	}
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Node Teaching Mode Queue | Node Teaching Mode Queue Count(%d)", get_teaching_mode_queue_count(node_teaching_mode_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

/**
 * @brief Function that takes care of adding NODE_TEACHING_MODE_START requests from cloud to a queue
 * @param none
 * @retval none
 */
void add_to_teaching_mode_queue()
{
	teaching_mode_t *teaching_mode_node = (teaching_mode_t *)malloc(sizeof(teaching_mode_t));
	if (teaching_mode_node != NULL)
		*teaching_mode_node = node_teaching_mode_t;
	else
	{
		red_printf(QUEUE_ERROR_TAG, "Memory allocation failed in add_to_teaching_mode_queue");
		return;
	}
	if (node_teaching_mode_queue_head == NULL)
	{
		// Adding the first element into the queue
		node_teaching_mode_queue_head = node_teaching_mode_queue_tail = teaching_mode_node;
		teaching_mode_node->next = teaching_mode_node->prev = NULL;
	}
	else
	{
		node_teaching_mode_queue_tail->next = teaching_mode_node;
		node_teaching_mode_queue_tail->next->prev = node_teaching_mode_queue_tail;
		node_teaching_mode_queue_tail->next->next = NULL;
		node_teaching_mode_queue_tail = teaching_mode_node;
	}
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Node Teaching Mode Queue | Node Teaching Mode Queue Count(%d)", get_teaching_mode_queue_count(node_teaching_mode_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

/**
 * @brief Get the node teaching mode queue count
 * @param none
 * @return Number of elements currently in node teaching mode queue
 */
uint8_t get_teaching_mode_queue_count(teaching_mode_t *head)
{
	uint8_t count = 0;
	while (head != NULL)
	{
		count++;
		head = head->next;
	}
	return count;
}

/**
 * @brief Function that takes care of the housekeeping work of clearing off elements from
 * queue when they have stayed in the queue for too long than the NODE_COMM_TIMEOUT specified.
 * @param none
 * @retval none
 */
void maintain_teaching_mode_queue()
{
	teaching_mode_t *temp = node_teaching_mode_queue_head;
	while (temp != NULL)
	{
		if (esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_COMM_TIMEOUT_INTERVAL_US)
		{
			sprintf(queue_log_buffer, "Removing teaching mode request(msgseqno : %ld) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d}",
					JSON_PACKET_ID_KEY, NODE_TEACHING_MODE_START_PACKET,
					JSON_ACK_NAME_KEY, NODE_TEACHING_MODE_START_ACK_NAME,
					MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
					GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
					NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
					ELEMENT_ADDR_KEY, temp->base_data.elementAddr,
					LOCATION_KEY, temp->base_data.location,
					ERROR_CODE_KEY, NODE_COMM_TIMEOUT);
			add_to_pubmesg_queue(pubmessage, publish_topic);
			remove_from_teaching_mode_queue();
		}
		temp = temp->next;
	}
}

/*-----------------------------------------------------------------------------------------*/

void remove_from_debug_info_queue()
{
	if (node_debug_info_queue_head == NULL)
	{
		red_printf(QUEUE_ERROR_TAG, "node teaching mode queue is empty");
		return;
	}
	else
	{
		debug_info_t *temp = node_debug_info_queue_head;
		node_debug_info_queue_head = node_debug_info_queue_head->next;
		free(temp);
		if (node_debug_info_queue_head == NULL)
			node_debug_info_queue_tail = NULL;
	}
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node removed from Node Debug Info Queue | Node Debug Info Queue Count(%d)", get_debug_info_queue_count(node_debug_info_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

void add_to_debug_info_queue()
{
	debug_info_t *debuf_info_node = (debug_info_t *)malloc(sizeof(debug_info_t));
	if (debuf_info_node != NULL)
		*debuf_info_node = node_debug_info_t;
	else
	{
		red_printf(QUEUE_ERROR_TAG, "Memory allocation failed in add_to_debug_info_queue");
		return;
	}
	if (node_debug_info_queue_head == NULL)
	{
		// Adding the first element into the queue
		node_debug_info_queue_head = debuf_info_node = debuf_info_node;
		debuf_info_node->next = debuf_info_node->prev = NULL;
	}
	else
	{
		node_debug_info_queue_tail->next = debuf_info_node;
		node_debug_info_queue_tail->next->prev = node_debug_info_queue_tail;
		node_debug_info_queue_tail->next->next = NULL;
		node_debug_info_queue_tail = debuf_info_node;
	}
	snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Node Debug Info Queue | Node Debug Info Queue Count(%d)", get_debug_info_queue_count(node_debug_info_queue_head));
	yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
}

uint8_t get_debug_info_queue_count(debug_info_t *head)
{
	uint8_t count = 0;
	while (head != NULL)
	{
		count++;
		head = head->next;
	}
	return count;
}

void maintain_debug_info_queue()
{
	debug_info_t *temp = node_debug_info_queue_head;
	while (temp != NULL)
	{
		if (esp_timer_get_time() - temp->base_data.request_in_time_us > NODE_COMM_TIMEOUT_INTERVAL_US)
		{
			sprintf(queue_log_buffer, "Removing Node Debug Info request(msgseqno : %ld) due to NODE_COMM_TIMEOUT ... ", temp->base_data.msg_seq_no);
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d}",
					JSON_PACKET_ID_KEY, NODE_DEBUG_INFO_PACKET,
					JSON_ACK_NAME_KEY, NODE_DEBUG_INFO_ACK_NAME,
					MSG_SEQ_NO_KEY, temp->base_data.msg_seq_no,
					GWY_SER_NO_KEY, temp->base_data.gwy_ser_no_str,
					NODE_SER_NO_KEY, temp->base_data.node_ser_no_str,
					ELEMENT_ADDR_KEY, temp->base_data.elementAddr,
					ERROR_CODE_KEY, NODE_COMM_TIMEOUT);
			add_to_debug_info_queue(pubmessage, publish_topic);
			remove_from_debug_info_queue();
		}
		temp = temp->next;
	}
}

/*-----------------------------------------------------------------------------------------*/

/**
 * @brief Get the pubmesg queue count
 * @param none
 * @return Number of elements currently in Pubmesg queue
 */
uint8_t get_pubmesg_queue_count(pubmesg_t *head)
{
	uint8_t count = 0;
	while (head != NULL)
	{
		count++;
		head = head->next;
	}
	return count;
}

void remove_from_pubmesg_queue()
{
	if (pubmesg_queue_head == NULL)
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
		if (pubmesg_queue_head == NULL)
			pubmesg_queue_tail = NULL;
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
	// If it's bad time to add to pubmesg due to LTE no response, then hold off and don't keep adding. This will lead to OOM eventually.
	if (!hold_adding_to_pubmesg)
	{
		struct pub_mesg_struct *pubmesg_node = (struct pub_mesg_struct *)malloc(sizeof(struct pub_mesg_struct));
		if (pubmesg_node == NULL)
		{
			sprintf(queue_log_buffer, "Memory allocation failed in add_to_pubmesg_queue\r\n");
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			return;
		}
		strcpy(pubmesg_node->message, msg);
		pubmesg_node->topic = topic;
		if (pubmesg_queue_head == NULL)
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
		snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Node added to Pubmesg Queue | Pubmesg Queue Count(%d)", get_pubmesg_queue_count(pubmesg_queue_head));
		yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
	}
}


/*-----------------------------------------------------------------------------------------*/


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
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(100));
		if (prov_queue_head != NULL)
		{
			maintain_prov_queue();
			// Don't keep sending the requests again and again
			if (prov_queue_head != NULL)
			{
				if (!prov_queue_head->base_data.request_sent_to_node_flag)
				{
					prov_queue_head->base_data.request_sent_to_node_flag = true;
					ESP_LOGI(QUEUE_DEBUG_TAG, "Sending Prov Request to Node");
					send_prov_packet_to_node(prov_queue_head);
				}
			}
		}
		if (unprov_queue_head != NULL)
		{
			maintain_unprov_queue();
			// Don't keep sending the requests again and again
			if (unprov_queue_head != NULL)
			{
				if (!unprov_queue_head->base_data.request_sent_to_node_flag)
				{
					unprov_queue_head->base_data.request_sent_to_node_flag = true;
					ESP_LOGI(QUEUE_DEBUG_TAG, "Sending UnProv Request to Node");
					send_unprov_packet_to_node(unprov_queue_head);
				}
			}
		}
		if (node_reconf_queue_head != NULL)
		{
			maintain_reconf_queue();
			// Don't keep sending the requests again and again
			if (node_reconf_queue_head != NULL)
			{
				if (!node_reconf_queue_head->base_data.request_sent_to_node_flag)
				{
					node_reconf_queue_head->base_data.request_sent_to_node_flag = true;
					ESP_LOGI(QUEUE_DEBUG_TAG, "Sending Reconf Request to Node");
					send_reconf_packet_to_node(node_reconf_queue_head);
				}
			}
		}
		if (node_ac_control_queue_head != NULL)
		{
			maintain_ac_control_queue();
			// Don't keep sending the requests again and again
			if (node_ac_control_queue_head != NULL)
			{
				if (!node_ac_control_queue_head->base_data.request_sent_to_node_flag)
				{
					node_ac_control_queue_head->base_data.request_sent_to_node_flag = true;
					ESP_LOGI(QUEUE_DEBUG_TAG, "Sending AC Control Request to Node");
					send_ac_control_packet_to_node(node_ac_control_queue_head);
				}
			}
		}
		if (node_pub_conf_queue_head != NULL)
		{
			maintain_heartbeat_pubconf_queue();
			// Don't keep sending the requests again and again
			if (node_pub_conf_queue_head != NULL)
			{
				if (!node_pub_conf_queue_head->base_data.request_sent_to_node_flag)
				{
					node_pub_conf_queue_head->base_data.request_sent_to_node_flag = true;
					ESP_LOGI(QUEUE_DEBUG_TAG, "Sending Pub Conf Request to Node");
					send_pub_conf_packet_to_node(node_pub_conf_queue_head);
				}
			}
		}
		if (node_teaching_mode_queue_head != NULL)
		{
			maintain_teaching_mode_queue();
			// Don't keep sending the requests again and again
			if (node_teaching_mode_queue_head != NULL)
			{
				if (!node_teaching_mode_queue_head->base_data.request_sent_to_node_flag)
				{
					node_teaching_mode_queue_head->base_data.request_sent_to_node_flag = true;
					ESP_LOGI(QUEUE_DEBUG_TAG, "Sending Teaching Mode Request to Node");
					send_teaching_mode_packet_to_node(node_pub_conf_queue_head);
				}
			}
		}
		if (node_debug_info_queue_head != NULL)
		{
			maintain_debug_info_queue();
			// Don't keep sending the requests again and again
			if (node_debug_info_queue_head != NULL)
			{
				if (!node_debug_info_queue_head->base_data.request_sent_to_node_flag)
				{
					node_debug_info_queue_head->base_data.request_sent_to_node_flag = true;
					ESP_LOGI(QUEUE_DEBUG_TAG, "Sending Debug Info Request to Node");
					send_debug_info_packet_to_node(node_debug_info_queue_head);
				}
			}
		}
	}
}