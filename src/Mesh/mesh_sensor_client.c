#include "mesh_main.h"

static void example_ble_mesh_sensor_client_cb(esp_ble_mesh_sensor_client_cb_event_t event,
                                              esp_ble_mesh_sensor_client_cb_param_t *param)
{
    esp_ble_mesh_node_t *node = NULL;

	uint8_t firstByte;
    ESP_LOGI(TAG, "Sensor client, event %u, addr 0x%04x", event, param->params->ctx.addr);

    if (param->error_code) {
        ESP_LOGE(TAG, "Send sensor client message failed (err %d)", param->error_code);
        return;
    }

    node = esp_ble_mesh_provisioner_get_node_with_addr(param->params->ctx.addr);
    if (!node) {
        ESP_LOGE(TAG, "Node 0x%04x not exists", param->params->ctx.addr);
        return;
    }

    switch (event) {
    case ESP_BLE_MESH_SENSOR_CLIENT_GET_STATE_EVT:
        switch (param->params->opcode) {
        case ESP_BLE_MESH_MODEL_OP_SENSOR_DESCRIPTOR_GET:
            ESP_LOGI(TAG, "Sensor Descriptor Status, opcode 0x%04x", param->params->ctx.recv_op);
            if (param->status_cb.descriptor_status.descriptor->len != ESP_BLE_MESH_SENSOR_SETTING_PROPERTY_ID_LEN &&
                param->status_cb.descriptor_status.descriptor->len % ESP_BLE_MESH_SENSOR_DESCRIPTOR_LEN) {
                ESP_LOGE(TAG, "Invalid Sensor Descriptor Status length %d", param->status_cb.descriptor_status.descriptor->len);
                return;
            }
            if (param->status_cb.descriptor_status.descriptor->len) {
                ESP_LOG_BUFFER_HEX("Sensor Descriptor", param->status_cb.descriptor_status.descriptor->data,
                    param->status_cb.descriptor_status.descriptor->len);
                /* If running with sensor server example, sensor client can get two Sensor Property IDs.
                 * Currently we use the first Sensor Property ID for the following demonstration.
                 */
                sensor_prop_id = param->status_cb.descriptor_status.descriptor->data[1] << 8 |
                                 param->status_cb.descriptor_status.descriptor->data[0];
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_GET:
            ESP_LOGI(TAG, "Sensor Cadence Status, opcode 0x%04x, Sensor Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.cadence_status.property_id);
            ESP_LOG_BUFFER_HEX("Sensor Cadence", param->status_cb.cadence_status.sensor_cadence_value->data,
                param->status_cb.cadence_status.sensor_cadence_value->len);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET:
            ESP_LOGI(TAG, "Sensor Settings Status, opcode 0x%04x, Sensor Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.settings_status.sensor_property_id);
            ESP_LOG_BUFFER_HEX("Sensor Settings", param->status_cb.settings_status.sensor_setting_property_ids->data,
                param->status_cb.settings_status.sensor_setting_property_ids->len);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_GET:
            ESP_LOGI(TAG, "Sensor Setting Status, opcode 0x%04x, Sensor Property ID 0x%04x, Sensor Setting Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.setting_status.sensor_property_id,
                param->status_cb.setting_status.sensor_setting_property_id);
            if (param->status_cb.setting_status.op_en) {
                ESP_LOGI(TAG, "Sensor Setting Access 0x%02x", param->status_cb.setting_status.sensor_setting_access);
                ESP_LOG_BUFFER_HEX("Sensor Setting Raw", param->status_cb.setting_status.sensor_setting_raw->data,
                    param->status_cb.setting_status.sensor_setting_raw->len);
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_GET:
            ESP_LOGI(TAG, "Sensor Status, opcode 0x%04x", param->params->ctx.recv_op);
            if (param->status_cb.sensor_status.marshalled_sensor_data->len) {
                ESP_LOG_BUFFER_HEX("Sensor Data", param->status_cb.sensor_status.marshalled_sensor_data->data,
                    param->status_cb.sensor_status.marshalled_sensor_data->len);
                uint8_t *data = param->status_cb.sensor_status.marshalled_sensor_data->data;
                uint16_t length = 0;
                for (; length < param->status_cb.sensor_status.marshalled_sensor_data->len; ) {
                    uint8_t fmt = ESP_BLE_MESH_GET_SENSOR_DATA_FORMAT(data);
                    uint8_t data_len = ESP_BLE_MESH_GET_SENSOR_DATA_LENGTH(data, fmt);
                    uint16_t prop_id = ESP_BLE_MESH_GET_SENSOR_DATA_PROPERTY_ID(data, fmt);
                    uint8_t mpid_len = (fmt == ESP_BLE_MESH_SENSOR_DATA_FORMAT_A ?
                                        ESP_BLE_MESH_SENSOR_DATA_FORMAT_A_MPID_LEN : ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN);
                    ESP_LOGI(TAG, "Format %s, length 0x%02x, Sensor Property ID 0x%04x",
                        fmt == ESP_BLE_MESH_SENSOR_DATA_FORMAT_A ? "A" : "B", data_len, prop_id);
                    if (data_len != ESP_BLE_MESH_SENSOR_DATA_ZERO_LEN) {
                        ESP_LOG_BUFFER_HEX("Sensor Data", data + mpid_len, data_len + 1);
                        length += mpid_len + data_len + 1;
                        data += mpid_len + data_len + 1;
                    } else {
                        length += mpid_len;
                        data += mpid_len;
                    }
                }
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_GET:
            ESP_LOGI(TAG, "Sensor Column Status, opcode 0x%04x, Sensor Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.column_status.property_id);
            ESP_LOG_BUFFER_HEX("Sensor Column", param->status_cb.column_status.sensor_column_value->data,
                param->status_cb.column_status.sensor_column_value->len);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SERIES_GET:
            ESP_LOGI(TAG, "Sensor Series Status, opcode 0x%04x, Sensor Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.series_status.property_id);
            ESP_LOG_BUFFER_HEX("Sensor Series", param->status_cb.series_status.sensor_series_value->data,
                param->status_cb.series_status.sensor_series_value->len);
            break;
        default:
            ESP_LOGE(TAG, "Unknown Sensor Get opcode 0x%04x", param->params->ctx.recv_op);
            break;
        }
        break;
    case ESP_BLE_MESH_SENSOR_CLIENT_SET_STATE_EVT:
        switch (param->params->opcode) {
        case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET:
            ESP_LOGI(TAG, "Sensor Cadence Status, opcode 0x%04x, Sensor Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.cadence_status.property_id);
            ESP_LOG_BUFFER_HEX("Sensor Cadence", param->status_cb.cadence_status.sensor_cadence_value->data,
                param->status_cb.cadence_status.sensor_cadence_value->len);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET:
            ESP_LOGI(TAG, "Sensor Setting Status, opcode 0x%04x, Sensor Property ID 0x%04x, Sensor Setting Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.setting_status.sensor_property_id,
                param->status_cb.setting_status.sensor_setting_property_id);
            if (param->status_cb.setting_status.op_en) {
                ESP_LOGI(TAG, "Sensor Setting Access 0x%02x", param->status_cb.setting_status.sensor_setting_access);
                ESP_LOG_BUFFER_HEX("Sensor Setting Raw", param->status_cb.setting_status.sensor_setting_raw->data,
                    param->status_cb.setting_status.sensor_setting_raw->len);
            }
            break;
        default:
            ESP_LOGE(TAG, "Unknown Sensor Set opcode 0x%04x", param->params->ctx.recv_op);
            break;
        }
        break;

     case ESP_BLE_MESH_SENSOR_CLIENT_PUBLISH_EVT:
    	ESP_LOGI(TAG, "ESP_BLE_MESH_SENSOR_CLIENT_PUBLISH_EVT!!");
        switch (param->params->opcode)
        {
			case ESP_BLE_MESH_MODEL_OP_SENSOR_STATUS:
				ESP_LOGI(TAG, "Sensor Status, opcode 0x%04x, SrcAddr: 0x%04x", param->params->ctx.recv_op, param->params->ctx.addr);
				if (param->status_cb.sensor_status.marshalled_sensor_data->len) {
					ESP_LOG_BUFFER_HEX("Sensor Data", param->status_cb.sensor_status.marshalled_sensor_data->data,
						param->status_cb.sensor_status.marshalled_sensor_data->len);

					uint8_t *data = param->status_cb.sensor_status.marshalled_sensor_data->data;
					uint16_t length = 0;
					uint8_t md = *(data+0);

					sensorClient_t.srcEleAddr = param->params->ctx.addr;
					ESP_LOGI(TAG, "MARSHALLED DATA LENGTH - %d", param->status_cb.sensor_status.marshalled_sensor_data->len);

					uint8_t uart_tx[20] = {0};
					switch (param->status_cb.sensor_status.marshalled_sensor_data->len) {
					case DSU_MD_LEN:
						ESP_LOGI(TAG, "RECEIVED DSU DATA!!!");
						setCommonUartTxData(uart_tx, sensorClient_t.srcEleAddr);
						uart_tx[1] = DSU_UNIT ^ 128;
						sensorClient_t.dsuData[0] = uart_tx[4] = (*(data + 2)) ^ 128;	//doorStatus
						uart_write_bytes(UART_NUM_2, (const char*)uart_tx, 20);

						uint8_t door_stat = uart_tx[4]^128;
						if (door_stat) ESP_LOGI(TAG, "Door status - %d - OPEN\r\n", door_stat);
						else ESP_LOGI(TAG, "Door status - %d - CLOSE\r\n", door_stat);
						break;
					case NH3_IAQ_MD_LEN:
						if(md == 0xE2) {
							ESP_LOGI(TAG, "RECEIVED IAQ DATA!!!");
							setCommonUartTxData(uart_tx, sensorClient_t.srcEleAddr);
							uart_tx[1] = IAQ_UNIT ^ 128;
							sensorClient_t.iaqData[0] = uart_tx[4] = (*(data+2)) ^ 128;	//iaq MSB
							sensorClient_t.iaqData[1] = uart_tx[5] = (*(data+3)) ^ 128; //iaq LSB
							uart_write_bytes(UART_NUM_2, (const char*)uart_tx, 20);

							uint16_t aqi = ((uart_tx[4]^128)<<8) | (uart_tx[5]^128);
							ESP_LOGI(TAG, "Air quality index - %d\r\n", aqi);
							break;
						} else {
							ESP_LOGI(TAG, "RECEIVED NH3 DATA!!!");
							setCommonUartTxData(uart_tx, sensorClient_t.srcEleAddr);
							uart_tx[1] = NH3_UNIT ^ 128;
							sensorClient_t.nh3Data[0] = uart_tx[4] = (*(data + 2)) ^ 128;	//nh3Conc MSB
							sensorClient_t.nh3Data[1] = uart_tx[5] = (*(data + 3)) ^ 128;	//nh3Conc LSB
							uart_write_bytes(UART_NUM_2, (const char*)uart_tx, 20);

							float nh3Ppm = (float)( (((uart_tx[4]^128)<<8) | (uart_tx[5]^128)) / 100.0);
							ESP_LOGI(TAG, "NH3 concentration in ppm - %f\r\n", nh3Ppm);
							break;
						}
					case PCU_LUX_MD_LEN:
						if(md == 0xE2) {
							ESP_LOGI(TAG, "RECEIVED PCU DATA!!!");
							setCommonUartTxData(uart_tx, sensorClient_t.srcEleAddr);
							uart_tx[1] = PCU_UNIT ^ 128;
							sensorClient_t.pcuData[0] = uart_tx[4] = (*(data + 2)) ^ 128;	//current MSB
							sensorClient_t.pcuData[1] = uart_tx[5] = (*(data + 3)) ^ 128;	//current LSB
							sensorClient_t.pcuData[2] = uart_tx[6] = (*(data + 6)) ^ 128;	//voltage MSB
							sensorClient_t.pcuData[3] = uart_tx[7] = (*(data + 7)) ^ 128;	//voltage LSB
							sensorClient_t.pcuData[4] = uart_tx[8] = (*(data + 10)) ^ 128;	//relayStatus
							uart_write_bytes(UART_NUM_2, (const char*)uart_tx, 20);

							uint32_t receivedCurr = ( ((uart_tx[4]^128)<<8) | (uart_tx[5]^128) );
							float ip_curr = (float) ((float)receivedCurr / 100.0);
							uint16_t ip_volt = ((uart_tx[6]^128)<<8) | (uart_tx[7]^128);
							uint8_t relay_stat = uart_tx[8] ^ 128;
							ESP_LOGI(TAG, "Ip current - %f, Ip voltage - %d, Relay state - %d\r\n", ip_curr, ip_volt, relay_stat);
						} else {
							ESP_LOGI(TAG, "RECEIVED LUX DATA!!!");
							setCommonUartTxData(uart_tx, sensorClient_t.srcEleAddr);
							uart_tx[1] = LUX_UNIT ^ 128;
							sensorClient_t.luxData[0] = uart_tx[4] = (*(data+2)) ^ 128;		//Lux MSB
							sensorClient_t.luxData[1] = uart_tx[5] = (*(data+3)) ^ 128;		//Lux LSB
							sensorClient_t.luxData[2] = uart_tx[6] = (*(data+10)) ^ 128;	//Btry perc
							sensorClient_t.luxData[2] = uart_tx[7] = (*(data+6)) ^ 128;		//btryVtg MSB
							sensorClient_t.luxData[2] = uart_tx[8] = (*(data+7)) ^ 128;		//btryVtg LSB
							uart_write_bytes(UART_NUM_2, (const char*)uart_tx, 20);

							uint16_t lux = ((uart_tx[4]^128)<<8) | (uart_tx[5]^128);
							uint8_t bat_perc_lux = uart_tx[10]^128;
							uint16_t bat_mv_lux	= ((uart_tx[6]^128)<<8) | (uart_tx[7]^128);
							ESP_LOGI(TAG, "Lux - %d, Battery percentage - %d, Battery voltage - %d\r\n", lux, bat_perc_lux, bat_mv_lux);
						}
						break;
					case THU_MD_LEN:
						ESP_LOGI(TAG, "RECEIVED THU DATA!!!");
						setCommonUartTxData(uart_tx, sensorClient_t.srcEleAddr);
						uart_tx[1] = THU_UNIT ^ 128;
						sensorClient_t.thuData[0] = uart_tx[4] = (*(data+2)) ^ 128;		//temp MSB
						sensorClient_t.thuData[1] = uart_tx[5] = (*(data+3)) ^ 128;		//temp LSB
						sensorClient_t.thuData[2] = uart_tx[6] = (*(data+6)) ^ 128;		//RH
						sensorClient_t.thuData[3] = uart_tx[7] = (*(data+13)) ^ 128;	//battery percentage
						sensorClient_t.thuData[4] = uart_tx[8] = (*(data+9)) ^ 128;		//battery mV MSB
						sensorClient_t.thuData[5] = uart_tx[9] = (*(data+10)) ^ 128;	//battery mV LSB

						uart_write_bytes(UART_NUM_2, (const char*)uart_tx, 20);

						float temp = (((uart_tx[4]^128)<<8) | (uart_tx[5]^128)) / 10;
						uint8_t rh = uart_tx[6]^128;
						uint8_t bat_perc_thu = uart_tx[7]^128;
						uint16_t bat_vtg_thu = ((uart_tx[8]^128)<<8) | (uart_tx[9]^128);
						ESP_LOGI(TAG, "Temperature - %.01f�C, R.Humidity - %d%%, Battery voltage - %d, Battery percentage - %d%%\r\n", temp, rh, bat_vtg_thu, bat_perc_thu);
						break;
					}
				}
				break;
        }
        break;
    case ESP_BLE_MESH_SENSOR_CLIENT_TIMEOUT_EVT:
        example_ble_mesh_sensor_timeout(param->params->opcode);
    default:
        break;
    }
}