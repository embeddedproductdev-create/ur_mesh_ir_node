# IR BLE Mesh AC Controller

The scope of the project is to develop a BLE Mesh based Universal AC controller which can be used to control Air Conditioners remotely.

- Project Start Date   : 14th December, 2023
- Project End Date     : 31st December, 2024

## Project Members

- **Project Manager** : Saravana Perumal, Narendhra Singh
- **Project Lead**    : Kulasekaran
- **Project Members** : Umamaheswari, Adhikesavan

## Releases

| No | Release Version | Release date | Author | Changes | Link To Binaries |
|:----:|:-----------------:|:--------------:|:--------:|:---------:|:------------------:|
|1|0.6|24.04.2024|Kulasekaran|1) Fixed Teaching mode bug - Umamaheswari|[Ver 0.6](https://qmaxltd-my.sharepoint.com/:f:/g/personal/embedded_qmaxsys_com/EsAfWCuAF5pDgIHuPiZqN0sB0QvNj8S_XqIxa7qbzuJ3xw?e=lQr8X0)|
|2|0.7|15.06.2024|Kulasekaran|1) Long run LTE issue workaround with Rebooting|[Ver 0.7](https://qmaxltd-my.sharepoint.com/:f:/g/personal/embedded_qmaxsys_com/ElOpzTQT0UdEvAXS_M6xHUoBCObxUryYEZ7gFOrPOhTN-Q?e=Iybtim)|
|3|0.8.1|28.06.2024|Kulasekaran|1) Location string length increased from 20 to 30  2) Temperature Data ACK converted into Heartbeat ACK    3) Minimum value for Heartbeat publish configuration increased from 5 to 10  3) Added Purple LED indication when sending out and IR signal  4) Added Teaching mode start Packets for both Gwy/Node  5) Added Teaching Mode End ACK for both Gwy and Node  6) Added Debug Info packets for both Gwy and Node  7) Error code values changed  8) LED indication change for Teaching mode and AC Remote configuration mode  9) Added BootUp LED indication  10) Modified Button Press Logics|[Ver 0.8.1](https://qmaxltd-my.sharepoint.com/:f:/g/personal/embedded_qmaxsys_com/EmEgsRgmN0pBqarVdK6FILoBhknFlkaPrFwbDWWqYjniaw?e=blzBsU)|
|4|0.8.5|08.07.2024|Kulasekaran|1) Fixed issue in AC not getting controlled  2) Fix for NodeSerNo not found in Prov ACK  3) Fix for unable to perform Teaching Mode|[Ver 0.8.5](http://git.qmaxsys.com/Unimation/c01230-ir-ble-mesh-ac-controller-gwy/archive/v0.8.5.zip)|
|5|0.8.6|10.07.2024|Kulasekaran|1) Fix for Manual Control ACK not appearing in v0.8.5  2)Fix for Gwy reboot issue  3)Fix for Node MacId missing in Node Prov Ack  4)Fix for Ambient temperature being 0 in Node Heartbeat ACK|[Ver 0.8.6](http://git.qmaxsys.com/Unimation/c01230-ir-ble-mesh-ac-controller-gwy/archive/v0.8.6.zip)|
|6|0.8.7|02.08.2024|Kulasekaran|1) LED indication for unsupported remotes during AC remote configuration process  2) NODE_ALREADY_PROV error code removed  3) Removed suspend & resume method to avoid assert fail during AC control  4) Removed separate thread used for sending IR commands|[Ver 0.8.7](http://git.qmaxsys.com/Unimation/c01230-ir-ble-mesh-ac-controller-gwy/releases/tag/v0.8.7)|
|7|0.8.8|23.07.2024|Kulasekaran|1) Added JSON_PACKET_INVALID errorcode  2) Default Heartbeat increased from 10s to 300s  3) OTA related changes  4) Fixed Teaching Mode End ACK not received from Node|[Ver 0.8.8](http://git.qmaxsys.com/Unimation/c01230-ir-ble-mesh-ac-controller-gwy/releases/tag/v0.8.7)|
|8|1.0.0|02.12.2024|Kulasekaran|1) Automatic provisioning  2) Teaching mode made more reliable  3) ErrorCodes modified  4) LED Indications changed  5) Button press functionalities changed|[Ver 1.0.0](http://git.qmaxsys.com/Unimation/c01230-ir-ble-mesh-ac-controller-gwy/commit/203d73dc9965f99c58371fa721d854e389ce116e)|
|9|1.0.1|03.12.2024|Kulasekaran|1) Bug Fixes with Node Teaching Mode  2) Other minor bug fixes|[Ver 1.0.1]()|

## High level overview

### High-Level Overview

This project is a **Smart AC Remote Control System** featuring a **Gateway** and **Node** architecture. The Gateway acts as the central hub, connecting to the internet via LTE and communicating with the Node devices through a BLE Mesh network. The system enables seamless control of air conditioners using MQTT topics.

- **AC Control**: Send IR commands to ACs based on supported protocols.
- **BLE Mesh Communication**: Gateway and Nodes interact through a robust BLE mesh for command relay.
- **Mobile App Integration**: Commands and status updates are exchanged between the app and devices via MQTT broker.
- **Teaching Mode**: Learn and configure new AC models that aren't supported by the IR Library.
- **Customizable Indications**: Physical buttons and RGB LEDs on devices provide user feedback for actions and system states.

This repository contains the firmware, configuration tools, and setup instructions for provisioning and deploying the system. It also includes detailed documentation on JSON packet structures, error codes, and MQTT topic definitions for developers integrating or extending functionality.

## Hardware Details

- [Schematics](https://qmaxltd-my.sharepoint.com/:b:/g/personal/embedded_qmaxsys_com/EcAzDj2xZpRPheBzz8MhQ4MBeOw5IVayl4XTD_MZNdEs2Q?e=pJdu1H)

## Software Documentation and other Helper documents

- Software Documentation: [Link to Software Documentation](https://qmaxltd-my.sharepoint.com/:b:/g/personal/embedded_qmaxsys_com/Ecg-9dGB5xhImCbBhZHVzpkBaUj-b9RqdxJE9BViXP49UQ?e=Z1EH2k)
- MCU : ESP32-S3 [Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- MCU : ESP32-S3 [Technical Reference manual](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
- LTE : [Quectel's EC200U Hardware design document](https://forums.quectel.com/uploads/short-url/j0qEXlvPl25PfUDBf4QEkc9AQyx.pdf)
- [AT commands manual](https://forums.quectel.com/uploads/short-url/dV5cK9eteeQmwyGPgfWB351oZde.pdf)
- [TCP/IP command manual](https://forums.quectel.com/uploads/short-url/1loXiVyd118F6AYTRvznNkCbh1L.pdf)
- [MQTT commands manual](https://auroraevernet.ru/upload/iblock/c81/rfhactu9l14ymr9cxt3pebdqxfu39h5v.pdf)

## Software Dependencies

- VS Code IDE
- ESP-IDF v5.1.2
- IRremoteESP8266 Library [Github Link](https://github.com/crankyoldgit/IRremoteESP8266)
- Python39 - To generate Serial Number bin files

## CustomModificiations

1. Alarm.c of ESP-IDF changed.
2. esp_hal gpio logs turned off
3. IRremoteESP8266 changed to include Daikin200 send functions
