# JSON Packets

## Introduction

This markdown file contains information relating to all the JSON packet formats used in IR BLE Mesh AC Controller project.

## Purpose

The documentation ensures a clear understanding of the JSON Packet and JSON Ack format used by the IR BLE Mesh AC Controller system, empowering developers and stakeholders to maintain, debug, or extend the system's functionality efficiently.

## Table of Contents

1. [Gateway Registration Packet](#gateway-registration-packet)
2. [Gateway Registration Ack](#gateway-registration-ack)
3. [Gateway AC Remote Configuration Ack](#gateway-ac-remote-configuration-ack)
4. [Gateway Unregistration Packet](#gateway-unregistration-packet)
5. [Gateway Unregistration Ack](#gateway-unregistration-ack)
6. [Gateway AC Control Packet](#gateway-ac-control-packet)
7. [Gateway AC Control Ack](#gateway-ac-control-ack)
8. [Gateway Manual AC Control Ack](#gateway-manual-ac-control-ack)
9. [Gateway AC Remote Reconfiguration Packet](#gateway-ac-remote-reconfiguration-packet)
10. [Gateway AC Remote Reconfiguration Ack](#gateway-ac-remote-reconfiguration-ack)
11. [Gateway Heartbeat Ack](#gateway-heartbeat-ack)
12. [Gateway Heartbeat Publish Configuration Packet](#gateway-heartbeat-publish-configuration-packet)
13. [Gateway Heartbeat Publish Configuration Ack](#gateway-heartbeat-publish-configuration-ack)
14. [Gateway Teaching Mode Packet](#gateway-teaching-mode-packet)
15. [Gateway Teaching Mode Ack](#gateway-teaching-mode-ack)
16. [Gateway Teaching Mode Command Selection Packet](#gateway-teaching-mode-command-selection-packet)
17. [Gateway Teaching Mode Command Selection Ack](gateway-teaching-mode-command-selection-ack)
18. [Gateway Debug Info Packet](#gateway-debug-info-packet)
19. [Gateway Debug Info Ack](#gateway-debug-info-ack)
20. [Gateway General Ack](#gateway-general-ack)
21. [Node Provisioning Ack](#node-provisioning-ack)
22. [Node AC Remote Configuration Ack](#node-ac-remote-configuration-ack)
23. [Node UnProvisioning Packet](#node-unprovisioning-packet)
24. [Node UnProvisioning Ack](#node-unprovisioning-ack)
25. [Node AC Control Packet](#node-ac-control-packet)
26. [Node AC Control Ack](#node-ac-control-ack)
27. [Node Manual AC Control Ack](#node-manual-ac-control-ack)
28. [Node AC Remote Reconfiguration Packet](#node-ac-remote-reconfiguration-packet)
29. [Node AC Remote Reconfiguration Ack](#node-ac-remote-reconfiguration-ack)
30. [Node Heartbeat Ack](#node-heartbeat-ack)
31. [Node Heartbeat Publish Configuration Packet](#node-heartbeat-publish-configuration-packet)
32. [Node Heartbeat Publish Configuration Ack](#node-heartbeat-publish-configuration-ack)
33. [Node Teaching Mode Packet](#node-teaching-mode-packet)
34. [Node Teaching Mode Command Selection Packet](node-teaching-mode-command-selection-packet)
35. [Node Teaching Mode Command Selection Ack](node-teaching-mode-command-selection-ack)
36. [Node Teaching Mode Ack](#node-teaching-mode-ack)
37. [Node Debug Info Packet](#node-debug-info-packet)
38. [Node Debug Info Ack](#node-debug-info-ack)
39. [Node General Ack](#node-general-ack)

---

## Gateway Registration Packet

- **Description**: This packet is used for registering a new Gateway device.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       0       |   Number   |      0      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    Location    |  "1st Floor"  |   String   |  20 chars   |

```json
{
  "JsonPacketId": 0,
  "MsgSeqNo": 12345,
  "Location": "1st Floor"
}
```

---

## Gateway Registration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       0       |   Number   |      0      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 0,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway AC Remote Configuration Ack

- **Description**: This ack is used to denote that the AC Remote configuration status of the Gateway device. This ack also contains the IrProtocol information which indicates, as which AC remote, the Gateway will be acting as.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       1       |   Number   |      1      |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   IrProtocol   |     "LG2"     |   String   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 1,
  "GwySerNo": "GWY00001",
  "IrProtocol": "LG2",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway Unregistration Packet

- **Description**: This packet is used for Unregistering a registered Gateway device. Upon unregistration, the Gateway device will be factory reset and will be restarted automatically.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       2       |   Number   |      2      |
|    MsgSeqNo    |     54321     |   Number   |   0-65535   |

```json
{
  "JsonPacketId": 2,
  "MsgSeqNo": 54321
}
```

---

## Gateway Unregistration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       2       |   Number   |      2      |
|    MsgSeqNo    |     54321     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 2,
  "MsgSeqNo": 54321,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway AC Control Packet

- **Description**: This packet is used for controlling an AC using the Gateway.

|  Parameter Name  | Example Value | Value Type |        Value Range         |
| :--------------: | :-----------: | :--------: | :------------------------: |
|   JsonPacketId   |       3       |   Number   |             3              |
|     MsgSeqNo     |     98765     |   Number   |          0-65535           |
|      Power       |       1       |   Number   |            0,1             |
|   Temperature    |      25       |   Number   |           18-32            |
|     FanSpeed     |       5       |   Number   |            0-5             |
|       Mode       |    "Cool"     |   string   | Cool, Heat, Fan, Auto, Dry |
|      SwingH      |       1       |   Number   |            0,1             |
|      SwingV      |       1       |   Number   |            0,1             |
|     OnTimer      |       0       |   Number   |            0-12            |
|     OffTimer     |       0       |   Number   |            0-12            |
|     Locking      |       1       |   Number   |            0,1             |
| TempLockUpLimit  |      18       |   Number   |           18-32            |
| TempLockLowLimit |      32       |   Number   |           18-32            |

```json
{
  "JsonPacketId": 3,
  "MsgSeqNo": 98765,
  "Power": 1,
  "Temperature": 25,
  "FanSpeed": 5,
  "Mode": "Cool",
  "SwingH": 1,
  "SwingV": 1,
  "OnTimer": 0,
  "OffTimer": 0,
  "Locking": 1,
  "TempLockUpLimit": 18,
  "TempLockLowLimit": 32
}
```

---

## Gateway AC Control Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       3       |   Number   |      3      |
|    MsgSeqNo    |     98765     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 3,
  "MsgSeqNo": 98765,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway Manual AC Control Ack

- **Description**: This ack is used for notifiying the user about any manual AC control activity that has happened.

|    Parameter Name    | Example Value | Value Type | Value Range |
| :------------------: | :-----------: | :--------: | :---------: |
|     JsonPacketId     |       4       |   Number   |      4      |
|       GwySerNo       |   GWY00001    |   string   |     N/A     |
|        Power         |       1       |   Number   |     0,1     |
|     Temperature      |      23       |   Number   |    18-32    |
|       FanSpeed       |       5       |   Number   |     0-5     |
|         Mode         |    "Cool"     |   string   |     N/A     |
|    PowerErrorCode    |       0       |   Number   |     N/A     |
| TemperatureErrorCode |       0       |   Number   |     N/A     |
|  FanspeedErrorCode   |       0       |   Number   |     N/A     |
|    ModeErrorCode     |       0       |   Number   |     N/A     |

```json
{
  "JsonPacketId": 4,
  "GwySerNo": "GWY00001",
  "Power": 1,
  "Temperature": 23,
  "FanSpeed": 5,
  "Mode": "Cool",
  "PowerErrorCode": 0,
  "TemperatureErrorCode": 0,
  "FanspeedErrorCode": 0,
  "ModeErrorCode": 0
}
```

---

## Gateway AC Remote Reconfiguration Packet

- **Description**: This packet is used for making the Gateway device unconfigured so that it can be configured with any other AC Remote if needed.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       5       |   Number   |      5      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |

```json
{
  "JsonPacketId": 5,
  "MsgSeqNo": 12345
}
```

---

## Gateway AC Remote Reconfiguration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       5       |   Number   |      5      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 5,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway Heartbeat Ack

- **Description**: This ack is used to denote the alivenss of the Gateway device. This ack will be sent automatically according to the PublishPeriodSec set using the [Gateway Heartbeat Publish Configuration Packet](#gateway-heartbeat-publish-configuration-packet). Default Heartbeat interval is 300s.

|       Parameter Name        | Example Value | Value Type |        Value Range         |
| :-------------------------: | :-----------: | :--------: | :------------------------: |
|        JsonPacketId         |       6       |   Number   |             6              |
|          GwySerNo           |   GWY00001    |   string   |            N/A             |
| AmbientTemperature(Digital) |      25       |   Number   |            N/A             |
| AmbientTemperature(Analog)  |      25       |   Number   |            N/A             |
|            Power            |       1       |   Number   |            0,1             |
|         Temperature         |      25       |   Number   |           18-32            |
|          FanSpeed           |       5       |   Number   |            0-5             |
|            Mode             |    "Cool"     |   string   | Cool, Heat, Fan, Auto, Dry |
|           SwingH            |       1       |   Number   |            0,1             |
|           SwingV            |       1       |   Number   |            0,1             |
|           OnTimer           |       0       |   Number   |            0-12            |
|          OffTimer           |       0       |   Number   |            0-12            |
|           Locking           |       1       |   Number   |            0,1             |
|       TempLockUpLimit       |      18       |   Number   |           18-32            |
|      TempLockLowLimit       |      32       |   Number   |           18-32            |

```json
{
  "JsonPacketId": 6,
  "GwySerNo": "GWY00001",
  "AmbientTemperature(Digital)": 25,
  "AmbientTemperature(Analog)": 25,
  "Power": 1,
  "Temperature": 25,
  "FanSpeed": 5,
  "Mode": "Cool",
  "SwingH": 1,
  "SwingV": 1,
  "OnTimer": 0,
  "OffTimer": 0,
  "Locking": 1,
  "TempLockUpLimit": 18,
  "TempLockLowLimit": 32
}
```

---

## Gateway Heartbeat Publish Configuration Packet

- **Description**: This packet is used to set the interval at which the Gateway should send its heartbeat ack.

|  Parameter Name  | Example Value | Value Type | Value Range |
| :--------------: | :-----------: | :--------: | :---------: |
|   JsonPacketId   |       7       |   Number   |      7      |
|     MsgSeqNo     |     12345     |   Number   |   0-65535   |
| PublishPeriodSec |      300      |   Number   |  300-65535  |

```json
{
  "JsonPacketId": 7,
  "MsgSeqNo": 12345,
  "PublishPeriodSec": 300
}
```

---

## Gateway Heartbeat Publish Configuration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       7       |   Number   |      7      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 7,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway Teaching Mode Packet

- **Description**: This packet is used to make a Gateway enter/exit the Teaching Mode. Teaching mode is the process of recording IR commands fired from AC remote and storing it in flash memory, so that it can be replayed to control the AC. This needs to be used for protocols that are not currently sendable by the library.
- **TeachingStart**: If this is set to "1", it will make the Gateway to enter Teaching Mode. If this is set to "0", then it will make the Gateway to exit Teaching Mode.
- **StartingTemperature**: The Starting temperature for the teaching mode process.
- **EndingTemperature**: The ending temperature for the teaching mode process.

|   Parameter Name    | Example Value | Value Type | Value Range |
| :-----------------: | :-----------: | :--------: | :---------: |
|    JsonPacketId     |       8       |   Number   |      8      |
|      MsgSeqNo       |     12345     |   Number   |   0-65535   |
|  ErrorCheckEnabled  |       1       |   Number   |     0,1     |
|    TeachingStart    |       1       |   Number   |     0,1     |
| StartingTemperature |      25       |   Number   |    18-32    |
|  EndingTemperature  |      28       |   Number   |    18-32    |

```json
{
  "JsonPacketId": 8,
  "MsgSeqNo": 12345,
  "ErrorCheckEnabled": 1,
  "TeachingStart": 1,
  "StartingTemperature": 25,
  "EndingTemperature": 28
}
```

---

## Gateway Teaching Mode Ack

|  Parameter Name   |         Example Value         | Value Type | Value Range |
| :---------------: | :---------------------------: | :--------: | :---------: |
|   JsonPacketId    |               8               |   Number   |      8      |
|     MsgSeqNo      |             12345             |   Number   |   0-65535   |
|     GwySerNo      |           GWY00001            |   string   |     N/A     |
|     ErrorCode     |               0               |   Number   |     N/A     |
|     ErrorMsg      |           "SUCCESS"           |   string   |     N/A     |
| RemainingCommands |               8               |   Number   |    2-16     |
|    LastCommand    | "Temperature - 18 Power - On" |   string   |     N/A     |
|    NextCommand    | "Temperature - 19 Power - On" |   string   |     N/A     |

```json
{
  "JsonPacketId": 8,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS",
  "RemainingCommands": 8,
  "LastCommand": "Temperature - 18 | Power - On",
  "NextCommand": "Temperature - 18 | Power - On"
}
```

---

## Gateway Teaching Mode Command Selection Packet

- **Description**: This packet is used to select the comamnd for which the IR signal needs to be recorded. This packet aids in overwriting the recorded command slots, in case a wrong command was sent by accident. This makes the teaching process for protocols that are fully unsupported by the library to be more robust and reliable.
- **Power**: If this is set to "0", then a power-off command is about to be recorded.
- **Temperature**: If this is set to 26, then the "Temperature 26 | Power ON" command is about to be recorded.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       9       |   Number   |      9      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|     Power      |       1       |   Number   |     0,1     |
|  Temperature   |      26       |   Number   |    18-32    |

```json
{
  "JsonPacketId": 9,
  "MsgSeqNo": 12345,
  "Power": 1,
  "Temperature": 26
}
```

---

## Gateway Teaching Mode Command Selection Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |       9       |   Number   |      9      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 9,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway Debug Info Packet

- **Description**: This packet is used to get useful information from the Gateway, like, How long has the Gateway been running, What firmware is running in the Gateway, etc., This can also be used to either remotely factory reset the Gateway or just remotely restart the Gateway.
- **ResetDevice**: Setting this to "1" and sending this packet will factory reset the Gateway device.
- **RestartDevice**: Setting this to "1" and sending this packet will restart the Gateway device.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      10       |   Number   |     10      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|  ResetDevice   |       0       |   Number   |     0,1     |
| RestartDevice  |       0       |   Number   |     0,1     |

```json
{
  "JsonPacketId": 10,
  "MsgSeqNo": 12345,
  "ResetDevice": 0,
  "RestartDevice": 0
}
```

---

## Gateway Debug Info Ack

|  Parameter Name  | Example Value | Value Type | Value Range |
| :--------------: | :-----------: | :--------: | :---------: |
|   JsonPacketId   |      10       |   Number   |     10      |
|     MsgSeqNo     |     12345     |   Number   |   0-65535   |
|     GwySerNo     |   GWY00001    |   string   |     N/A     |
|    ErrorCode     |       0       |   Number   |     N/A     |
|     ErrorMsg     |   "SUCCESS"   |   string   |     N/A     |
| FirmwareVersion  |    "1.0.0"    |   string   |     N/A     |
|    Registered    |   "SUCCESS"   |   Number   |     N/A     |
|    IrProtocol    |     "LG2"     |   string   |     N/A     |
| PublishPeriodSec |      300      |   Number   |     N/A     |
| DeviceUpTimeHrs  |     "0.5"     |   string   |     N/A     |

```json
{
  "JsonPacketId": 10,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS",
  "FirmwareVersion": "1.0.0",
  "Registered": 1,
  "IrProtocol": "LG2",
  "PublishPeriodSec": 300,
  "DeviceUpTimeHrs": "0.5"
}
```

---

## Gateway General Ack

- **Description**: This ack is used to notify reg. corner error cases that may happen during runtime, like device running out of memory, failed to initialize a partition, etc.,

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      12       |   Number   |     12      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 12,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Node Provisioning Ack

- **Description**: This ack is sent by the Gateway, when it provisions a Node. The **ElementAddr** parameter in the ack is a very crucial parameter, that needs to be noted down for any further communication with the provisioned Node device.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      100      |   Number   |     100     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 100,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Node AC Remote Configuration Ack

- **Description**: This ack is used to denote that the AC Remote configuration status of the Node device. This ack also contains the IrProtocol information which indicates, as which AC remote, the Node will be acting as.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      101      |   Number   |     101     |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   IrProtocol   |     "LG2"     |   String   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 101,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "IrProtocol": "LG2",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Node Unprovisioning Packet

- **Description**: This packet is used to Unprovision a Node device. Once unprovisioned, the Node will stop advertising in-order to avoid getting reprovisioned by the Gateway immediately. In case, the same Node wants to be provisioned, it simly needs to be restarted to start its advertising.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      102      |   Number   |     102     |
|    MsgSeqNo    |     54321     |   Number   |   0-65535   |
|  ElementAddr   |      23       |   Number   |   2-65535   |

```json
{
  "JsonPacketId": 102,
  "MsgSeqNo": 54321,
  "ElementAddr": 23
}
```

---

## Node Unprovisioning Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      102      |   Number   |     102     |
|    MsgSeqNo    |     54321     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 102,
  "MsgSeqNo": 54321,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Node AC Control Packet

- **Description**: This packet is used to control an AC using Node.

|  Parameter Name  | Example Value | Value Type |        Value Range         |
| :--------------: | :-----------: | :--------: | :------------------------: |
|   JsonPacketId   |      103      |   Number   |            103             |
|     MsgSeqNo     |     98765     |   Number   |          0-65535           |
|   ElementAddr    |      23       |   Number   |          2-65535           |
|      Power       |       1       |   Number   |            0,1             |
|   Temperature    |      25       |   Number   |           18-32            |
|     FanSpeed     |       5       |   Number   |            0-5             |
|       Mode       |    "Cool"     |   string   | Cool, Heat, Fan, Auto, Dry |
|      SwingH      |       1       |   Number   |            0,1             |
|      SwingV      |       1       |   Number   |            0,1             |
|     OnTimer      |       0       |   Number   |            0-12            |
|     OffTimer     |       0       |   Number   |            0-12            |
|     Locking      |       1       |   Number   |            0,1             |
| TempLockUpLimit  |      18       |   Number   |           18-32            |
| TempLockLowLimit |      32       |   Number   |           18-32            |

```json
{
  "JsonPacketId": 103,
  "MsgSeqNo": 98765,
  "ElementAddr": 23,
  "Power": 1,
  "Temperature": 25,
  "FanSpeed": 5,
  "Mode": "Cool",
  "SwingH": 1,
  "SwingV": 1,
  "OnTimer": 0,
  "OffTimer": 0,
  "Locking": 1,
  "TempLockUpLimit": 18,
  "TempLockLowLimit": 32
}
```

---

## Node AC Control Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      103      |   Number   |     103     |
|    MsgSeqNo    |     98765     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 103,
  "MsgSeqNo": 98765,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Node Manual AC Control Ack

- **Description**: This ack is used for notifiying the user about any manual AC control activity that has happened.

|    Parameter Name    | Example Value | Value Type | Value Range |
| :------------------: | :-----------: | :--------: | :---------: |
|     JsonPacketId     |      104      |   Number   |     104     |
|       GwySerNo       |   GWY00001    |   string   |     N/A     |
|      NodeSerNo       |    N00001     |   string   |     N/A     |
|     ElementAddr      |      23       |   Number   |   2-65535   |
|        Power         |       1       |   Number   |     0,1     |
|     Temperature      |      23       |   Number   |    18-32    |
|       FanSpeed       |       5       |   Number   |     0-5     |
|         Mode         |    "Cool"     |   string   |     N/A     |
|    PowerErrorCode    |       0       |   Number   |     N/A     |
| TemperatureErrorCode |       0       |   Number   |     N/A     |
|  FanspeedErrorCode   |       0       |   Number   |     N/A     |
|    ModeErrorCode     |       0       |   Number   |     N/A     |

```json
{
  "JsonPacketId": 104,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "Power": 1,
  "Temperature": 23,
  "FanSpeed": 5,
  "Mode": "Cool",
  "PowerErrorCode": 0,
  "TemperatureErrorCode": 0,
  "FanspeedErrorCode": 0,
  "ModeErrorCode": 0
}
```

---

## Node AC Remote Reconfiguration Packet

- **Description**: This packet is used for making the Node device unconfigured so that it can be configured with any other AC Remote if needed.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      105      |   Number   |     105     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|  ElementAddr   |      23       |   Number   |   2-65535   |

```json
{
  "JsonPacketId": 105,
  "MsgSeqNo": 12345,
  "ElementAddr": 23
}
```

---

## Node AC Remote Reconfiguration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      105      |   Number   |     105     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 105,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Node Heartbeat Ack

- **Description**: This ack is used to denote the alivenss of the Node device. This ack will be sent automatically according to the PublishPeriodSec set using the [Node Heartbeat Publish Configuration Packet](#node-heartbeat-publish-configuration-packet). Default Heartbeat interval is 300s.

|       Parameter Name        | Example Value | Value Type |        Value Range         |
| :-------------------------: | :-----------: | :--------: | :------------------------: |
|        JsonPacketId         |      106      |   Number   |            106             |
|          GwySerNo           |   GWY00001    |   string   |            N/A             |
|          NodeSerNo          |    N00001     |   string   |            N/A             |
|         ElementAddr         |      23       |   Number   |          2-65535           |
| AmbientTemperature(Digital) |      25       |   Number   |            N/A             |
| AmbientTemperature(Analog)  |      25       |   Number   |            N/A             |
|            Power            |       1       |   Number   |            0,1             |
|         Temperature         |      25       |   Number   |           18-32            |
|          FanSpeed           |       5       |   Number   |            0-5             |
|            Mode             |    "Cool"     |   string   | Cool, Heat, Fan, Auto, Dry |
|           SwingH            |       1       |   Number   |            0,1             |
|           SwingV            |       1       |   Number   |            0,1             |
|           OnTimer           |       0       |   Number   |            0-12            |
|          OffTimer           |       0       |   Number   |            0-12            |
|           Locking           |       1       |   Number   |            0,1             |
|       TempLockUpLimit       |      18       |   Number   |           18-32            |
|      TempLockLowLimit       |      32       |   Number   |           18-32            |

```json
{
  "JsonPacketId": 106,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "AmbientTemperature(Digital)": 25,
  "AmbientTemperature(Analog)": 25,
  "Power": 1,
  "Temperature": 25,
  "FanSpeed": 5,
  "Mode": "Cool",
  "SwingH": 1,
  "SwingV": 1,
  "OnTimer": 0,
  "OffTimer": 0,
  "Locking": 1,
  "TempLockUpLimit": 18,
  "TempLockLowLimit": 32
}
```

---

## Node Heartbeat Publish Configuration Packet

- **Description**: This packet is used to set the interval at which the Node should send its heartbeat ack.

|  Parameter Name  | Example Value | Value Type | Value Range |
| :--------------: | :-----------: | :--------: | :---------: |
|   JsonPacketId   |      107      |   Number   |     107     |
|     MsgSeqNo     |     12345     |   Number   |   0-65535   |
|   ElementAddr    |      23       |   Number   |   2-65535   |
| PublishPeriodSec |      300      |   Number   |  300-65535  |

```json
{
  "JsonPacketId": 107,
  "MsgSeqNo": 12345,
  "ElementAddr": 23,
  "PublishPeriodSec": 300
}
```

---

## Node Heartbeat Publish Configuration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      107      |   Number   |     107     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 107,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Node Teaching Mode Packet

- **Description**: This packet is used to make a Node enter/exit the Teaching Mode. Teaching mode is the process of recording IR commands fired from AC remote and storing it in flash memory, so that it can be replayed to control the AC. This needs to be used for protocols that are not currently sendable by the library.
- **TeachingStart**: If this is set to "1", it will make the Node to enter Teaching Mode. If this is set to "0", then it will make the Node to exit Teaching Mode.
- **StartingTemperature**: The Starting temperature for the teaching mode process.
- **EndingTemperature**: The ending temperature for the teaching mode process.

|   Parameter Name    | Example Value | Value Type | Value Range |
| :-----------------: | :-----------: | :--------: | :---------: |
|    JsonPacketId     |      108      |   Number   |     108     |
|      MsgSeqNo       |     12345     |   Number   |   0-65535   |
|     ElementAddr     |      23       |   Number   |   2-65535   |
|  ErrorCheckEnabled  |       1       |   Number   |     0,1     |
|    TeachingStart    |       1       |   Number   |     0,1     |
| StartingTemperature |      25       |   Number   |    18-32    |
|  EndingTemperature  |      28       |   Number   |    18-32    |

```json
{
  "JsonPacketId": 108,
  "MsgSeqNo": 12345,
  "ElementAddr": 23,
  "ErrorCheckEnabled": 1,
  "TeachingStart": 1,
  "StartingTemperature": 25,
  "EndingTemperature": 28
}
```

---

## Node Teaching Mode Ack

|  Parameter Name   |         Example Value         | Value Type | Value Range |
| :---------------: | :---------------------------: | :--------: | :---------: |
|   JsonPacketId    |              108              |   Number   |     108     |
|     MsgSeqNo      |             12345             |   Number   |   0-65535   |
|     GwySerNo      |           GWY00001            |   string   |     N/A     |
|     NodeSerNo     |            N00001             |   string   |     N/A     |
|    ElementAddr    |              23               |   Number   |   2-65535   |
|     ErrorCode     |               0               |   Number   |     N/A     |
|     ErrorMsg      |           "SUCCESS"           |   string   |     N/A     |
| RemainingCommands |               8               |   Number   |    2-16     |
|    LastCommand    | "Temperature - 18 Power - On" |   string   |     N/A     |
|    NextCommand    | "Temperature - 19 Power - On" |   string   |     N/A     |

```json
{
  "JsonPacketId": 108,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS",
  "RemainingCommands": 8,
  "LastCommand": "Temperature - 18 | Power - On",
  "NextCommand": "Temperature - 18 | Power - On"
}
```

---

## Node Teaching Mode Command Selection Packet

- **Description**: This packet is used to select the comamnd for which the IR signal needs to be recorded. This packet aids in overwriting the recorded command slots, in case a wrong command was sent by accident. This makes the teaching process for protocols that are fully unsupported by the library to be more robust and reliable.
- **Power**: If this is set to "0", then a power-off command is about to be recorded.
- **Temperature**: If this is set to 26, then the "Temperature 26 | Power ON" command is about to be recorded.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      109      |   Number   |     109     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|     Power      |       1       |   Number   |     0,1     |
|  Temperature   |      26       |   Number   |    18-32    |

```json
{
  "JsonPacketId": 109,
  "MsgSeqNo": 12345,
  "ElementAddr": 23,
  "Power": 1,
  "Temperature": 26
}
```

---

## Node Teaching Mode Command Selection Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      109      |   Number   |     109     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 109,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Node Debug Info Packet

- **Description**: This packet is used to get useful information from the Node, like, How long has the Node been running, What firmware is running in the Node, etc., This can also be used to either remotely factory reset the Node or just remotely restart the Node.
- **ResetDevice**: Setting this to "1" and sending this packet will factory reset the Node device.
- **RestartDevice**: Setting this to "1" and sending this packet will restart the Node device.

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      110      |   Number   |     110     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|  ResetDevice   |       0       |   Number   |     0,1     |
| RestartDevice  |       0       |   Number   |     0,1     |

```json
{
  "JsonPacketId": 110,
  "MsgSeqNo": 12345,
  "ElementAddr": 23,
  "ResetDevice": 0,
  "RestartDevice": 0
}
```

---

## Node Debug Info Ack

|  Parameter Name  | Example Value | Value Type | Value Range |
| :--------------: | :-----------: | :--------: | :---------: |
|   JsonPacketId   |      110      |   Number   |     110     |
|     MsgSeqNo     |     12345     |   Number   |   0-65535   |
|     GwySerNo     |   GWY00001    |   string   |     N/A     |
|    NodeSerNo     |    N00001     |   string   |     N/A     |
|   ElementAddr    |      23       |   Number   |   2-65535   |
|    ErrorCode     |       0       |   Number   |     N/A     |
|     ErrorMsg     |   "SUCCESS"   |   string   |     N/A     |
| FirmwareVersion  |    "1.0.0"    |   string   |     N/A     |
|    Registered    |   "SUCCESS"   |   Number   |     N/A     |
|    IrProtocol    |     "LG2"     |   string   |     N/A     |
| PublishPeriodSec |      300      |   Number   |     N/A     |
| DeviceUpTimeHrs  |     "0.5"     |   string   |     N/A     |

```json
{
  "JsonPacketId": 110,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "NodeSerNo": "N00001",
  "ElementAddr": 23,
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS",
  "FirmwareVersion": "1.0.0",
  "Registered": 1,
  "IrProtocol": "LG2",
  "PublishPeriodSec": 300,
  "DeviceUpTimeHrs": "0.5"
}
```

---

## Node General Ack

- **Description**: This ack is used to notify reg. corner error cases that may happen during runtime, like device running out of memory, failed to initialize a partition, etc.,

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|  JsonPacketId  |      112      |   Number   |     112     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "JsonPacketId": 112,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---
