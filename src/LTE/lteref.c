void LTE_task()
{
    uint8_t zero_flag = 0;
    uint8_t cons_zero = 0;
    uint8_t i = 0;
    uint8_t cid = 0;

    master_init();

    esp_err_t err = ESP_OK;
    float value = 0;
    bool alarm_state = false;
    const mb_parameter_descriptor_t *param_descriptor = NULL;

    mb_param_request_t param_request = {};
    uint8_t ret;
    uart_init();

    localLte_t.state = LED_SERVER_NOT_CONNECTED;
    uint8_t count = 0;
    do
    {
        if (count > 10)
        {
            resetLte();
        }
        ret = OT_command("ATE0\r\n");
        if (ret == SUCCESS)
            ret = 1;
        else
            ret = 0;
        count++;
    } while (!ret);

    vTaskDelay(pdMS_TO_TICKS(10));

    err = MQTT_Config(CLIENT_IDX, 0, 0, 120, 1, 0, 1, 1, 0, 0, mqttWillTopic, "offline");

    if (err == FAILURE)
        esp_restart();

    vTaskDelay(10);
    do
    {
        ret = MQTT_NetworkOpen(CLIENT_IDX, localMqtt_t.host, localMqtt_t.port);
        if (ret == 1)
            ESP_LOGI(TAG, "Connected to network successfully");
        else if (ret == 2)
        {
            ESP_LOGI(TAG, "Closing network!!!");
            MQTT_NetworkClose(CLIENT_IDX);
        }
        else
            ESP_LOGI(TAG, "Could not connect to network!!");
    } while (ret != 1);

    vTaskDelay(pdMS_TO_TICKS(1000));

    ret = MQTT_NetworkOpenCheck();

    localLte_t.state = LED_CLIENT_NOT_CONNECTED;

    vTaskDelay(pdMS_TO_TICKS(1000));

    do
    {
        ret = MQTT_ClientConnect(CLIENT_IDX, localMqtt_t.user, localMqtt_t.pwd, deviceSerNo);
        if (ret == 1)
            ESP_LOGI(TAG, "Connected to client successfully!!!");
        else if (ret == CLOSE_CONNECTION)
        {
            ESP_LOGI(TAG, "Closing network!!!");
            vTaskDelay(pdMS_TO_TICKS(500));
            MQTT_NetworkClose(CLIENT_IDX);
            vTaskDelay(pdMS_TO_TICKS(500));
            MQTT_NetworkOpen(CLIENT_IDX, localMqtt_t.host, localMqtt_t.port);
        }
        else
            ESP_LOGI(TAG, "Cannot connect to client!!!");
    } while (ret != 1);

    vTaskDelay(pdMS_TO_TICKS(1000));

    ret = MQTT_ClientConnectCheck();

    localLte_t.state = LED_CLIENT_CONNECTED;

    vTaskDelay(pdMS_TO_TICKS(1000));

    do
    {
        ret = SubscribeTopic(CLIENT_IDX, 2, mqttSubTopic, 0);
        if (ret == 1)
            ESP_LOGI(TAG, "Subscribed to topic successfully!!!");
        else
            ESP_LOGI(TAG, "Cannot subscribbe to topic!!!");
    } while (ret != 1);

    ESP_ERROR_CHECK(esp_timer_start_periodic(hb_timer, 1000000));  // Start heartbeat timer once the device is connected to cloud
    ESP_ERROR_CHECK(esp_timer_start_periodic(emr_timer, 1000000)); // 1s timer
    char *rxData = "";
    while (1)
    {
        ESP_LOGI(TAG, "Internal RAM - %d", esp_get_free_internal_heap_size());
        if (nmTimer_t.hbCount >= 55) // Send Heartbeat to cloud
        {
            lte_sendHb();
            nmTimer_t.hbCount = 0;
        }

        rxData = ReadMessage(3);
        if (rxData != NULL)
        {
            if (strlen(rxData) > 40)
            {
                localLte_t.state = LED_ACTIVITY;
                ESP_LOGI(TAG, "Going to parse!!!");
                uint8_t whatTopic = lte_determineTopic(rxData);
                char *payload = lte_extractPayload(rxData);
                lte_doDesiredProcess(whatTopic, payload);
            }
        }

        if (masterLocal_t.totalSlaves != 0 && masterLocal_t.emrSlaves != 0)
        {
            //			ESP_LOGI(TAG, "Task running!!!");
            for (i = 0; i < MAX_SLAVES_COUNT && emrLocal_t[i].present == 1; i++)
            {
                //				ESP_LOGI(TAG, "Checking for timer elapsed in slave #%d", i);
                zero_flag = 0;
                if (nmTimer_t.emrCount[i] >= (emrLocal_t[i].pubPeriod - 3))
                {
                    ESP_LOGI(TAG, "EMR slave #%d timer expired!!!", i);
                    for (uint16_t cid = EM_WATTS_TOTAL; cid < EM_MAX; cid++)
                    {
                        char actual_data[9] = {0};
                        void *data = malloc((sizeof(char) * 9));
                        if (emrLocal_t[i].pars_t[cid].regAvail)
                        {
                            param_request.slave_addr = emrLocal_t[i].slaveId;
                            param_request.command = 0x03;
                            param_request.reg_start = emrLocal_t[i].pars_t[cid].regOffset - 1;
                            param_request.reg_size = emrLocal_t[i].pars_t[cid].regLength;
                            //	    							ESP_LOGI(TAG, "Parameter -  #%d | Slave addr - %d | Command - %d | Start address - %d | Size - %d", cid, param_request.slave_addr, param_request.command, param_request.reg_start, param_request.reg_size);
                            err = mbc_master_send_request(&param_request, data);
                            if (err != ESP_OK)
                            {
                                ESP_LOGE(TAG, "Failed to read data from slave!!!");
                                zero_flag = 1;
                                emrLocal_t[i].pars_t[cid].read = 0;
                                //				        		ESP_ERROR_CHECK(err);
                            }
                            else
                            {
                                char *first_lsb = data + 0;
                                char *first_msb = data + 1;
                                char *sec_lsb = data + 2;
                                char *sec_msb = data + 3;

                                uint16_t first_byte = (*first_msb << 8) | *first_lsb;
                                uint16_t sec_byte = (*sec_msb << 8) | *sec_lsb;
                                uint32_t raw_data = sec_byte << 16 | first_byte;
                                //	    				       		float converted_data = ieeeToFloating(sec_byte, first_byte);
                                ESP_LOGI(TAG, "SlaveId - %d | Parameter - %d | Raw - 0x%x ", emrLocal_t[i].slaveId, cid, raw_data);
                                emrLocal_t[i].pars_t[cid].read = 1;
                                emrLocal_t[i].parValues[cid] = raw_data;
                                if ((emrLocal_t[i].parValues[cid] <= 0) && (cons_zero < 4))
                                {
                                    zero_flag = 1;
                                    cons_zero++;
                                    cid--;
                                }
                                else
                                {
                                    zero_flag = 0;
                                    cons_zero = 0;
                                }
                            }
                        }
                        else
                        {
                            continue;
                        }
                        vTaskDelay(pdMS_TO_TICKS(50));
                        free(data);
                        data = NULL;
                    }
                    nmTimer_t.emrCount[i] = 0;
                    ESP_LOGI(TAG, "after reading from slave - Internal RAM - %d", esp_get_free_internal_heap_size());
                    lte_publishEmrData(i, DEVICE_EMR, emrLocal_t[i].parValues);
                }
            }
        }
        vTaskDelay(100);

        //	            vTaskDelay(100 / portTICK_PERIOD_MS);
        //	            free(emData);
        //	        }
    }
    vTaskDelete(NULL);
}