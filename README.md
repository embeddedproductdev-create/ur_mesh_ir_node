

# What is c0027_amenos_sdk
---

The **c0027_amenos_sdk** provides with C ++ arduino libraries to build an product  Amenos which is an IOT connected device used for controlling AC and lighting devices in rooms. 

# What it does
---


The main functionality is to controlling Room lighting and Air Conditioners which may be of either Legacy type HVAC or Split-AC. . It uses Wi-Fi and BLE for communication.RGB Leds,Touch switches,Buzzer,Dot Matrixis used for user interface.

# Getting started
---

## Prerequisites: 
***

- Install [Arduino IDE](arduino.cc/en/Main/Software) 
- Amenos target board designed by Qmax

## Installing ESP32 Add-on in Arduino IDE
***
To install the ESP32 board in Arduino IDE, follow the instructions [here](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)

## Download SDK
***
Download  [c0027_amenos_sdk](http://gerrit.qmaxonline.com/login/)
> Only Qmax Employes can clone the SDK

## Install Libray
***
- After downloading sdk go to *lib* directory and copy all library listed below

    1. ArduinoJson
    2. ESP32_FTPClient-master
    3. Inspiron_AC
    4. IRremoteESP8266-master
    5. LedControl-master
    6. LTR303-master
    7. NTPClient
    8. Si1133

- paste it in default ESP library directory 
```
C:\Users\<user_name>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\libraries
```
## Build and compile SDK
***
Plug the ESP32 board to your computer. With your Arduino IDE open, follow these steps 

- Go to *C:\Users\<user_name>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\tools\partitions*
- Open *default_8MB.csv* and update partition table as below


| # Name | Type | SubType | Offset | Size | Flags |
| ------ | ------ | ------ | ------ | ------ | ------ |
| nvs | data | nvs | 0x9000 | 0x5000 |
| otadata | data | ota | 0xe000 | 0x2000 |
| app0 | app | ota_0 | 0x10000 | 0x300000 |
| app1 | app | ota_1 |  0x310000 | 0x225000 |
| spiffs | data | spiffs | 0x535000 | 0x225000 |

- Go to *C:\Users\<user_name>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4*
- Open *board.txt* and make change as below
```
esp32.menu.PartitionScheme.default_8MB=8M Flash (3MB APP/1.7MB FAT)
esp32.menu.PartitionScheme.default_8MB.build.partitions=default_8MB
esp32.menu.PartitionScheme.default_8MB.upload.maximum_size=3145728
```

- Open *afi_sdk.ino* file in arduino ide
- Go to Tools and make settings as below

```
Board : "ESP32 Dev Module"
Upload Speed: "921600"
CPU Frequency: "240MHz(WiFi/BT)"
Flash Frequency: "80MHz"
Flash Mode: "QIO"
Flash Size: "8MB (64Mb)"
Partition Scheme: "8M Flash (3MB APP/1.7MB FAT)"
Core Debug Level: "None"
PSRAM: "Disabled"
Port: <Select_com_port>
```
- Then Verify and Upload the Sketch to target device.After flashing to target will get log as below

```
Writing at 0x00008000... (100 %)
Wrote 3072 bytes (130 compressed) at 0x00008000 in 0.0 seconds (effective 2457.6 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
```
## UDP server running procedure
***
- Install [Python](https://www.python.org/downloads/)
- copy below python code and save it as *Udp_server.py*
```python
import socket

# bind all IP
HOST = '0.0.0.0' 
# Listen on Port 
PORT = 8080
#Size of receive buffer   
BUFFER_SIZE = 1024    
# Create a TCP/IP socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Bind the socket to the host and port
s.bind((HOST, PORT))
while True:
    # Receive BUFFER_SIZE bytes data
    # data is a list with 2 elements
    # first is data
    #second is client address
    data = s.recvfrom(BUFFER_SIZE)
    if data:
        #print received data
        print('Client to Server: ' , data)
        s.sendto(data[0], data[1])
# Close connection
s.close()
```
- Go to python installed directory in command prompt
```
cd C:\Python38
```
- Execute Udp server runing command 
```
python.exe  Path\TO\Python file\*.py
```
- Now Udp server will run in PC
## Test Procedure
***
- Use mobile for internet connectivity by Changing mobile *hotspot name* and *password* as below
```
    hotspot name - 123456
    password     - #123456123#
```
- Connect PC/Laptop and Target board to same network
- Run UDP server in windows PC
- Open command prompt and run the command *ipconfig*
```
Wireless LAN adapter Wi-Fi:

   Connection-specific DNS Suffix  . :
   IPv6 Address. . . . . . . . . . . : 2409:4072:6e90:676f:2cc4:56ed:62:7238
   Temporary IPv6 Address. . . . . . : 2409:4072:6e90:676f:91fa:c9f3:573d:e2ad
   Link-local IPv6 Address . . . . . : fe80::2cc4:56ed:62:7238%21
   IPv4 Address. . . . . . . . . . . : 192.168.43.18
   Subnet Mask . . . . . . . . . . . : 255.255.255.0
   Default Gateway . . . . . . . . . : fe80::9440:48ff:fe48:e312%21
                                       192.168.43.1
```
- Copy the IPv4 Address (eg.192.168.43.18) under *Wireless LAN adapter Wi-Fi*
- open *c0027_amenos_sdk.ino* in Arduino IDE and paste the IPv4 address to varibale *udp_ip* and set port as below
```csharp
const char* udp_ip = "192.168.43.18"; // enter pc ip
unsigned int udp_port = 8080;      // server port
```
- Download [BLE Scanner](https://play.google.com/store/apps/details?id=com.macdom.ble.blescanner&hl=en_IN) android app.
- Connect to ble device named as *CaleidoXenia*
- Configure the target device by sending following 4 packets
```
1405F4004358414D454E4F530000000000000000
1407F40131323331323331323334353601020101
1407F40231323334353637383931323334353637
1407F403414D454E4F5331323334350504000000
1407F40431323334353637380000000000000000
```
- After sending this target device will get internet connectivity and send health status to UDP server in *json* format
```json
{
  "hmac": "",
  "payload": {
  "device_uid": "1234566778",
  "data_type": 1,
  "device_data": [
    {
      "device_health_status": 0,
      "device_temperature":29.76
    }
  ]
  }
}
```

# Reference
***
- For software design structure and api reference refer *doc* directory

# Known issues
---
- ESP is resetting when external [Watch Dog Timer](https://www.digikey.in/product-detail/en/texas-instruments/TPS3852G33QDRBRQ1/296-45530-1-ND/6823160) is placed.

> When initializing BLE , ESP is resetting



