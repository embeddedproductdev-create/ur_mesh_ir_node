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
16. [Gateway Debug Info Packet](#gateway-debug-info-packet)
17. [Gateway Debug Info Ack](#gateway-debug-info-ack)
18. [Gateway General Ack](#gateway-general-ack)
19. [Node Provisioning Ack](#node-provisioning-ack)
20. [Node AC Remote Configuration Ack](#node-ac-remote-configuration-ack)
21. [Node UnProvisioning Packet](#node-unprovisioning-packet)
22. [Node UnProvisioning Ack](#node-unprovisioning-ack)
23. [Node AC Control Packet](#node-ac-control-packet)
24. [Node AC Control Ack](#node-ac-control-ack)
25. [Node Manual AC Control Ack](#node-manual-ac-control-ack)
26. [Node AC Remote Reconfiguration Packet](#node-ac-remote-reconfiguration-packet)
27. [Node AC Remote Reconfiguration Ack](#node-ac-remote-reconfiguration-ack)
28. [Node Heartbeat Ack](#node-heartbeat-ack)
29. [Node Heartbeat Publish Configuration Packet](#node-heartbeat-publish-configuration-packet)
30. [Node Heartbeat Publish Configuration Ack](#node-heartbeat-publish-configuration-ack)
31. [Node Teaching Mode Packet](#node-teaching-mode-packet)
32. [Node Teaching Mode Ack](#node-teaching-mode-ack)
33. [Node Debug Info Packet](#node-debug-info-packet)
34. [Node Debug Info Ack](#node-debug-info-ack)
35. [Node General Ack](#node-general-ack)

---

## Gateway Registration Packet

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       0       |   Number   |      0      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    Location    |  "1st Floor"  |   String   |  20 chars   |

```json
{
  "PacketId": 0,
  "MsgSeqNo": 12345,
  "Location": "1st Floor"
}
```

---

## Gateway Registration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       0       |   Number   |      0      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 0,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway AC Remote Configuration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       1       |   Number   |      1      |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   IrProtocol   |     "LG2"     |   String   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 1,
  "GwySerNo": "GWY00001",
  "IrProtocol": "LG2",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway Unregistration Packet

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       2       |   Number   |      2      |
|    MsgSeqNo    |     54321     |   Number   |   0-65535   |

```json
{
  "PacketId": 2,
  "MsgSeqNo": 54321
}
```

---

## Gateway Unregistration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       2       |   Number   |      2      |
|    MsgSeqNo    |     54321     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 2,
  "MsgSeqNo": 54321,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway AC Control Packet

|  Parameter Name  | Example Value | Value Type |        Value Range         |
| :--------------: | :-----------: | :--------: | :------------------------: |
|     PacketId     |       3       |   Number   |             3              |
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
  "PacketId": 3,
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
|    PacketId    |       3       |   Number   |      3      |
|    MsgSeqNo    |     98765     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 3,
  "MsgSeqNo": 98765,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway Manual AC Control Ack

|    Parameter Name    | Example Value | Value Type | Value Range |
| :------------------: | :-----------: | :--------: | :---------: |
|       PacketId       |       4       |   Number   |      4      |
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
  "PacketId": 4,
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

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       5       |   Number   |      5      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |

```json
{
  "PacketId": 5,
  "MsgSeqNo": 12345
}
```

---

## Gateway AC Remote Reconfiguration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       5       |   Number   |      5      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 5,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway Heartbeat Ack

|       Parameter Name        | Example Value | Value Type |        Value Range         |
| :-------------------------: | :-----------: | :--------: | :------------------------: |
|          PacketId           |       6       |   Number   |             6              |
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
  "PacketId": 6,
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

|  Parameter Name  | Example Value | Value Type | Value Range |
| :--------------: | :-----------: | :--------: | :---------: |
|     PacketId     |       7       |   Number   |      7      |
|     MsgSeqNo     |     12345     |   Number   |   0-65535   |
| PublishPeriodSec |      300      |   Number   |  300-65535  |

```json
{
  "PacketId": 7,
  "MsgSeqNo": 12345,
  "PublishPeriodSec": 300,
}
```

---

## Gateway Heartbeat Publish Configuration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       7       |   Number   |      7      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 7,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Gateway Teaching Mode Packet

|   Parameter Name    | Example Value | Value Type | Value Range |
| :-----------------: | :-----------: | :--------: | :---------: |
|      PacketId       |       8       |   Number   |      8      |
|      MsgSeqNo       |     12345     |   Number   |   0-65535   |
|    TeachingStart    |       1       |   Number   |     0,1     |
| StartingTemperature |      25       |   Number   |    18-32    |
|  EndingTemperature  |      28       |   Number   |    18-32    |

```json
{
  "PacketId": 8,
  "MsgSeqNo": 12345,
  "TeachingStart": 1,
  "StartingTemperature": 25,
  "EndingTemperature": 28
}
```

---

## Gateway Teaching Mode Ack

|  Parameter Name   |         Example Value         | Value Type | Value Range |
| :---------------: | :---------------------------: | :--------: | :---------: |
|     PacketId      |               8               |   Number   |      8      |
|     MsgSeqNo      |             12345             |   Number   |   0-65535   |
|     GwySerNo      |           GWY00001            |   string   |     N/A     |
|     ErrorCode     |               0               |   Number   |     N/A     |
|     ErrorMsg      |           "SUCCESS"           |   string   |     N/A     |
| RemainingCommands |               8               |   Number   |    2-16     |
|    LastCommand    | "Temperature - 18 Power - On" |   string   |     N/A     |
|    NextCommand    | "Temperature - 19 Power - On" |   string   |     N/A     |

```json
{
  "PacketId": 8,
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

## Gateway Debug Info Packet

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       9       |   Number   |      9      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|  ResetDevice   |       0       |   Number   |     0,1     |
| RestartDevice  |       0       |   Number   |     0,1     |

```json
{
  "PacketId": 9,
  "MsgSeqNo": 12345,
  "ResetDevice": 0,
  "RestartDevice": 0
}
```

---

## Gateway Debug Info Ack

|  Parameter Name  | Example Value | Value Type | Value Range |
| :--------------: | :-----------: | :--------: | :---------: |
|     PacketId     |       9       |   Number   |      9      |
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
  "PacketId": 9,
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

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      11       |   Number   |     11      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 11,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---

## Node Provisioning Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      100      |   Number   |     100     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 100,
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

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      101      |   Number   |     101     |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   IrProtocol   |     "LG2"     |   String   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 101,
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

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      102      |   Number   |     102     |
|    MsgSeqNo    |     54321     |   Number   |   0-65535   |
|  ElementAddr   |      23       |   Number   |   2-65535   |

```json
{
  "PacketId": 102,
  "MsgSeqNo": 54321,
  "ElementAddr": 23
}
```

---

## Node Unprovisioning Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      102      |   Number   |     102     |
|    MsgSeqNo    |     54321     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 102,
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

|  Parameter Name  | Example Value | Value Type |        Value Range         |
| :--------------: | :-----------: | :--------: | :------------------------: |
|     PacketId     |      103      |   Number   |            103             |
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
  "PacketId": 103,
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
|    PacketId    |      103      |   Number   |     103     |
|    MsgSeqNo    |     98765     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 103,
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

|    Parameter Name    | Example Value | Value Type | Value Range |
| :------------------: | :-----------: | :--------: | :---------: |
|       PacketId       |      104      |   Number   |     104     |
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
  "PacketId": 104,
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

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      105      |   Number   |     105     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|  ElementAddr   |      23       |   Number   |   2-65535   |

```json
{
  "PacketId": 105,
  "MsgSeqNo": 12345,
  "ElementAddr": 23
}
```

---

## Node AC Remote Reconfiguration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      105      |   Number   |     105     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 105,
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

|       Parameter Name        | Example Value | Value Type |        Value Range         |
| :-------------------------: | :-----------: | :--------: | :------------------------: |
|          PacketId           |      106      |   Number   |            106             |
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
  "PacketId": 106,
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

|  Parameter Name  | Example Value | Value Type | Value Range |
| :--------------: | :-----------: | :--------: | :---------: |
|     PacketId     |      107      |   Number   |     107     |
|     MsgSeqNo     |     12345     |   Number   |   0-65535   |
|   ElementAddr    |      23       |   Number   |   2-65535   |
| PublishPeriodSec |      300      |   Number   |  300-65535  |

```json
{
  "PacketId": 107,
  "MsgSeqNo": 12345,
  "ElementAddr": 23,
  "PublishPeriodSec": 300
}
```

---

## Node Heartbeat Publish Configuration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      107      |   Number   |     107     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   NodeSerNo    |    N00001     |   string   |     N/A     |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 107,
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

|   Parameter Name    | Example Value | Value Type | Value Range |
| :-----------------: | :-----------: | :--------: | :---------: |
|      PacketId       |      108      |   Number   |     108     |
|      MsgSeqNo       |     12345     |   Number   |   0-65535   |
|     ElementAddr     |      23       |   Number   |   2-65535   |
|    TeachingStart    |       1       |   Number   |     0,1     |
| StartingTemperature |      25       |   Number   |    18-32    |
|  EndingTemperature  |      28       |   Number   |    18-32    |

```json
{
  "PacketId": 108,
  "MsgSeqNo": 12345,
  "ElementAddr": 23,
  "TeachingStart": 1,
  "StartingTemperature": 25,
  "EndingTemperature": 28
}
```

---

## Node Teaching Mode Ack

|  Parameter Name   |         Example Value         | Value Type | Value Range |
| :---------------: | :---------------------------: | :--------: | :---------: |
|     PacketId      |              108              |   Number   |     108     |
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
  "PacketId": 108,
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

## Node Debug Info Packet

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      109      |   Number   |     109     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|  ElementAddr   |      23       |   Number   |   2-65535   |
|  ResetDevice   |       0       |   Number   |     0,1     |
| RestartDevice  |       0       |   Number   |     0,1     |

```json
{
  "PacketId": 109,
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
|     PacketId     |      109      |   Number   |     109     |
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
  "PacketId": 109,
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

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      111      |   Number   |     111     |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |     N/A     |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 111,
  "MsgSeqNo": 12345,
  "GwySerNo": "GWY00001",
  "ErrorCode": 0,
  "ErrorMsg": "SUCCESS"
}
```

---
