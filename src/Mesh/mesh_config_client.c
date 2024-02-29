#include "mesh_main.h"

static void example_ble_mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event,
                                              esp_ble_mesh_cfg_client_cb_param_t *param)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_cfg_client_set_state_t set = {0};
    static uint16_t wait_model_id, wait_cid;
    esp_ble_mesh_node_t *node = NULL;
    esp_err_t err = ESP_OK;

    uint8_t ackUartTx[20] = {0};

    ESP_LOGI(TAG, "Config client, event %u, addr 0x%04x, opcode 0x%04x",
        event, param->params->ctx.addr, param->params->opcode);

    if (param->error_code) {
        ESP_LOGE(TAG, "Send config client message failed (err %d)", param->error_code);
        return;
    }

    node = esp_ble_mesh_provisioner_get_node_with_addr(param->params->ctx.addr);
    if (!node) {
        ESP_LOGE(TAG, "Node 0x%04x not exists", param->params->ctx.addr);
        return;
    }

    switch (event) {
    case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET) {
            ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data,
                param->status_cb.comp_data_status.composition_data->len);
            example_ble_mesh_parse_node_comp_data(param->status_cb.comp_data_status.composition_data->data,
                param->status_cb.comp_data_status.composition_data->len);
            err = esp_ble_mesh_provisioner_store_node_comp_data(param->params->ctx.addr,
                param->status_cb.comp_data_status.composition_data->data,
                param->status_cb.comp_data_status.composition_data->len);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to store node composition data");
                break;
            }

            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
            set.app_key_add.net_idx = prov_key.net_idx;
            set.app_key_add.app_idx = prov_key.app_idx;
            memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to send Config AppKey Add");
            }
        }
        break;
    case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT:
    	if (provisioner_t.pcuUnitToBeProv) /* Rvd provision frame - to provision PCU */
    	{
			if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD) {
					example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
					set.model_app_bind.element_addr = node->unicast_addr;
					set.model_app_bind.model_app_idx = prov_key.app_idx;
					set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
					set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
					err = esp_ble_mesh_config_client_set_state(&common, &set);
					if (err != ESP_OK) {
						ESP_LOGE(TAG, "Failed to send Config Model App Bind");
						return;
					}
					wait_model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
					wait_cid = ESP_BLE_MESH_CID_NVAL;
			} else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND) {
				if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_SENSOR_SRV &&
					param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL) {
					example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
					set.model_app_bind.element_addr = node->unicast_addr;
					set.model_app_bind.model_app_idx = prov_key.app_idx;
					set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV;
					set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
					err = esp_ble_mesh_config_client_set_state(&common, &set);
					if (err) {
						ESP_LOGE(TAG, "Failed to send Config Model App Bind");
						return;
					}
					wait_model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV;
					wait_cid = ESP_BLE_MESH_CID_NVAL;
				} else if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV &&
					param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL) {
					example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
					set.model_app_bind.element_addr = node->unicast_addr;
					set.model_app_bind.model_app_idx = prov_key.app_idx;
					set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_SRV;
					set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
					err = esp_ble_mesh_config_client_set_state(&common, &set);
					if (err) {
						ESP_LOGE(TAG, "Failed to send Config Model App Bind");
						return;
					}
					set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_SRV;
					set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
				} else if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_SRV &&
						param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL) {
						ESP_LOGW(TAG, "Provisioning and Configuration successful!!");
    					setAckUartCommon(ackUartTx, node->unicast_addr, PROV_ACK, ACK_SUCCESS, NA);
    					uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
    					uint8_t mac[2] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    					esp_ble_mesh_provisioner_set_dev_uuid_match(mac, sizeof(mac), 0x6, true);
				}
            }
    	} else if (provisioner_t.rc7UnitToBeProv) /* Rvd provision frame - to provision RC7 */
		{
			if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD)
			{
					example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
					set.model_app_bind.element_addr = node->unicast_addr;
					set.model_app_bind.model_app_idx = prov_key.app_idx;
					set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_SRV;
					set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
					err = esp_ble_mesh_config_client_set_state(&common, &set);
					if (err != ESP_OK)
					{
						ESP_LOGE(TAG, "Failed to send Config Model App Bind");
						return;
					}
					wait_model_id = ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_SRV;
					wait_cid = ESP_BLE_MESH_CID_NVAL;
			} else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND)
			{
				ESP_LOGW(TAG, "Provisioning and Configuration successful!!!");
				setAckUartCommon(ackUartTx, node->unicast_addr, PROV_ACK, ACK_SUCCESS, NA);
				uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
				uint8_t mac[2] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
				esp_ble_mesh_provisioner_set_dev_uuid_match(mac, sizeof(mac), 0x6, true);
			}
		}  /* Rvd provision frame - to provision except RC7 & PCU */
		else
    	{
    			if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD)
    			{
    				example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
    				set.model_app_bind.element_addr = node->unicast_addr;
    				set.model_app_bind.model_app_idx = prov_key.app_idx;
    				set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
    				set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
    				err = esp_ble_mesh_config_client_set_state(&common, &set);
    				if (err != ESP_OK)
    				{
    					ESP_LOGE(TAG, "Failed to send Config Model App Bind");
    					return;
    				}
    				wait_model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
    				wait_cid = ESP_BLE_MESH_CID_NVAL;
    			}
    			else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND)
    			{
    				if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_SENSOR_SRV &&
    					param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL) {
    					example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
    					set.model_app_bind.element_addr = node->unicast_addr;
    					set.model_app_bind.model_app_idx = prov_key.app_idx;
    					set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV;
    					set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
    					err = esp_ble_mesh_config_client_set_state(&common, &set);
    					if (err)
    					{
    						ESP_LOGE(TAG, "Failed to send Config Model App Bind");
    						return;
    					}
    					wait_model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV;
    					wait_cid = ESP_BLE_MESH_CID_NVAL;
    				} else if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV &&
    					param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL)
    				{
    					ESP_LOGW(TAG, "Provisioning and Configuration successful!!!");
    					setAckUartCommon(ackUartTx, node->unicast_addr, PROV_ACK, ACK_SUCCESS, NA);
    					uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
    					uint8_t mac[2] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    					esp_ble_mesh_provisioner_set_dev_uuid_match(mac, sizeof(mac), 0x6, true);
    					if(proxy_t.thuOrLux)
    					{
    						err = esp_ble_mesh_proxy_client_disconnect(proxy_t.handle);
    						proxy_t.handle = 0;
    						proxy_t.connected = proxy_t.thuOrLux = 0;
    					}
    				}
    			}
            } /* End of pcuUnitToBeProv */
    	 if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET) {
    		  ESP_LOGI(TAG, "SET - PUBLISH ADDRESS SET!!, address - 0x%04x", param->params->ctx.addr);
    		  setAckUartCommon(ackUartTx, param->params->ctx.addr, MDL_CFG_PUB_ACK, ACK_SUCCESS, NA);
    		  uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
    	 } if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD) {
    		  ESP_LOGI(TAG, "SET - SUBSCRIBE ADDRESS ADDED!!, address - 0x%04x", param->params->ctx.addr);
    		  setAckUartCommon(ackUartTx, param->params->ctx.addr, MDL_CFG_SUB_ACK, ACK_SUCCESS, NA);
    		  uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
    	 } if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_NODE_RESET) {
			  ESP_LOGI(TAG, "SET - NODE UNPROVISION SUCCESSFUL!!, address - 0x%04x", param->params->ctx.addr);
			  setAckUartCommon(ackUartTx, param->params->ctx.addr, PROVDEL_ACK, ACK_SUCCESS, NA);
			  uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
    	 } if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_RELAY_SET) {
    		 ESP_LOGI(TAG, "SET - NODE RELAY EN_DIS SUCCESSFUL!!!, address - 0x%04x", param->params->ctx.addr);
    		 setAckUartCommon(ackUartTx, param->params->ctx.addr, RELENDIS_ACK, ACK_SUCCESS, NA);
    		 uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
    	 }
        break;
    case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_STATUS) {
            ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data,
                param->status_cb.comp_data_status.composition_data->len);
        } else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_PUB_STATUS) {
    		ESP_LOGI(TAG, "PUBLISH - PUBLISH ADDRESS SET ACK!!");
  		  setAckUartCommon(ackUartTx, param->params->ctx.addr, MDL_CFG_PUB_ACK, ACK_SUCCESS, NA);
  		  uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
		} else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD) {
    		ESP_LOGI(TAG, "PUBLISH - SUBSCRIBE ADDRESS ADDED!!");
  		  setAckUartCommon(ackUartTx, param->params->ctx.addr, MDL_CFG_SUB_ACK, ACK_SUCCESS, NA);
    		uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
		}
        break;
    case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
        switch (param->params->opcode) {
        case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET: {
        	++provConfigCount_t.compDataGet;
        	if(provConfigCount_t.compDataGet <= 7)
        	{
				esp_ble_mesh_cfg_client_get_state_t get = {0};
				example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
				get.comp_data_get.page = COMP_DATA_PAGE_0;
				err = esp_ble_mesh_config_client_get_state(&common, &get);
				if (err != ESP_OK) {
					ESP_LOGE(TAG, "Failed to send Config Composition Data Get");
				}
        	}
        	else
        	{
        		provConfigCount_t.appkeyAdd = provConfigCount_t.compDataGet = provConfigCount_t.modelAppBind = 0;
        		ESP_LOGW(TAG, "COMP_DATA_GET_TIMEOUT - Aborting provisioning!!!");
        	}
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
        	++provConfigCount_t.appkeyAdd;
        	if(provConfigCount_t.appkeyAdd <= 7)
        	{
				example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
				set.app_key_add.net_idx = prov_key.net_idx;
				set.app_key_add.app_idx = prov_key.app_idx;
				memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
				err = esp_ble_mesh_config_client_set_state(&common, &set);
				if (err != ESP_OK)
				{
					ESP_LOGE(TAG, "Failed to send Config AppKey Add");
				}
        	}
        	else
        	{
        		provConfigCount_t.appkeyAdd = provConfigCount_t.compDataGet = provConfigCount_t.modelAppBind = 0;
        		ESP_LOGW(TAG, "APPKEY_ADD_TIMEOUT - Aborting provisioning!!!");
        	}
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
        	++provConfigCount_t.modelAppBind;
        	if(provConfigCount_t.modelAppBind <= 7)  // 7 Retries
        	{
				example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
				set.model_app_bind.element_addr = node->unicast_addr;
				set.model_app_bind.model_app_idx = prov_key.app_idx;
				set.model_app_bind.model_id = wait_model_id;
				set.model_app_bind.company_id = wait_cid;
				err = esp_ble_mesh_config_client_set_state(&common, &set);
				if (err != ESP_OK)
				{
					ESP_LOGE(TAG, "Failed to send Config Model App Bind");
				}
        	}
        	else
        	{
        		switch(wait_model_id)
        		{
        			case ESP_BLE_MESH_MODEL_ID_SENSOR_SRV:
        				ESP_LOGI(TAG, "MODEL_APP_BIND_SENSOR_SRV_TIMEOUT - Aborting provisioning!!!");
        				provConfigCount_t.compDataGet = provConfigCount_t.appkeyAdd = provConfigCount_t.modelAppBind = 0;
        				break;
        			case ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV:
        				ESP_LOGI(TAG, "MODEL_APP_BIND_SENSOR_SETUP_SRV_TIMEOUT - Aborting provisioning!!!");
        				provConfigCount_t.compDataGet = provConfigCount_t.appkeyAdd = provConfigCount_t.modelAppBind = 0;
        				break;
        			case ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_SRV:
        				ESP_LOGI(TAG, "MODEL_APP_BIND_ONOFF_SRV_TIMEOUT - Aborting provisioning!!!");
        				provConfigCount_t.compDataGet = provConfigCount_t.appkeyAdd = provConfigCount_t.modelAppBind = 0;
        				break;
        		}
        	}
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET:
        	ESP_LOGI(TAG, "TIMEOUT SET- MODEL PUBLICATION STATE!!!");
        	setAckUartCommon(ackUartTx, param->params->ctx.addr, MDL_CFG_PUB_ACK, ACK_FAIL, NA);
        	uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
        	break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
        	ESP_LOGI(TAG, "TIMEOUT SET- ADD SUBSCRIBE ADDRESS!!!");
        	setAckUartCommon(ackUartTx, param->params->ctx.addr, MDL_CFG_SUB_ACK, ACK_FAIL, NA);
        	uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
        	break;
        case ESP_BLE_MESH_MODEL_OP_RELAY_SET:
        	ESP_LOGI(TAG, "TIMEOUT SET - RELAY ENABLE DISABLE!!!, address - 0x%04x", param->params->ctx.addr);
			setAckUartCommon(ackUartTx, param->params->ctx.addr, RELENDIS_ACK, ACK_FAIL, NA);
        	uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
        	break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_PUB_STATUS:
        	ESP_LOGI(TAG, "TIMEOUT STATUS - MODEL PUBLICATION STATE!!! ");
        	break;
        case ESP_BLE_MESH_MODEL_OP_NODE_RESET:
        	ESP_LOGI(TAG, "TIMEOUT RESET - UNPROVISION TIMEOUT!!!");
        	setAckUartCommon(ackUartTx, param->params->ctx.addr, PROVDEL_ACK, ACK_FAIL, NA);
        	uart_write_bytes(ackUartTx, (const char*)ackUartTx, 20);
        	break;
        default:
            break;
        }
        break;
    default:
        ESP_LOGE(TAG, "Invalid config client event %u", event);
        break;
    }
}