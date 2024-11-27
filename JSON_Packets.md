# Table of Contents

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

## Gateway Registration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       0       |   Number   |      0      |
|    MsgSeqNo    |     12345     |   Number   |   0-65535   |
|    GwySerNo    |   GWY00001    |   string   |     N/A     |
|   ErrorCode    |       0       |   Number   |  -1 to 67   |
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

## Gateway AC Remote Configuration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       1       |   Number   |      1      |
|   IrProtocol   |     "LG2"     |   String   |     N/A     |
|   ErrorCode    |       0       |   Number   |  -1 to 67   |
|    ErrorMsg    |   "SUCCESS"   |   string   |     N/A     |

```json
{
  "PacketId": 2,
  "MsgSeqNo": 54321,
  "Status": "Success"
}
```

## Gateway Unregistration Packet

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       1       |   Number   |      1      |
|    MsgSeqNo    |     54321     |   Number   |   0-65535   |
|    Location    |  "2nd Floor"  |   String   |  20 chars   |

```json
{
  "PacketId": 1,
  "MsgSeqNo": 54321,
  "Location": "2nd Floor"
}
```

## Gateway Unregistration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       1       |   Number   |      1      |
|    MsgSeqNo    |     54321     |   Number   |   0-65535   |
|   ErrorCode    |       0       |   Number   |  -1 to 67   |

```json
{
  "PacketId": 1,
  "MsgSeqNo": 54321,
  "ErrorCode": 0,
  "ErrorMessage": "SUCCESS"
}
```

## Gateway AC Control Packet

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       3       |   Number   |      3      |
|    MsgSeqNo    |     98765     |   Number   |   0-65535   |
|    Command     |  "Power On"   |   String   |  20 chars   |

```json
{
  "PacketId": 3,
  "MsgSeqNo": 98765,
  "Command": "Power On"
}
```

## Gateway AC Control Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       3       |   Number   |      3      |
|    MsgSeqNo    |     98765     |   Number   |   0-65535   |
|   ErrorCode    |       0       |   Number   |  -1 to 67   |

```json
{
  "PacketId": 3,
  "MsgSeqNo": 98765,
  "ErrorCode": 0,
  "ErrorMessage": "SUCCESS"
}
```

## Gateway Manual AC Control Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       4       |   Number   |      4      |
|    MsgSeqNo    |    112233     |   Number   |   0-65535   |
|    Command     |   "Cooling"   |   String   |  10 chars   |

```json
{
  "PacketId": 4,
  "MsgSeqNo": 112233,
  "Command": "Cooling"
}
```

## Gateway AC Remote Reconfiguration Packet

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       5       |   Number   |      5      |
|    MsgSeqNo    |    334455     |   Number   |   0-65535   |
|   NewConfig    |  "Config X"   |   String   |  20 chars   |

```json
{
  "PacketId": 5,
  "MsgSeqNo": 334455,
  "NewConfig": "Config X"
}
```

## Gateway AC Remote Reconfiguration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       5       |   Number   |      5      |
|    MsgSeqNo    |    334455     |   Number   |   0-65535   |
|     Status     |   "Success"   |   String   |  10 chars   |

```json
{
  "PacketId": 5,
  "MsgSeqNo": 334455,
  "Status": "Success"
}
```

## Gateway Heartbeat Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       6       |   Number   |      6      |
|    MsgSeqNo    |    445566     |   Number   |   0-65535   |
|     Status     |    "Alive"    |   String   |  10 chars   |

```json
{
  "PacketId": 6,
  "MsgSeqNo": 445566,
  "Status": "Alive"
}
```

## Gateway Heartbeat Publish Configuration Packet

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       7       |   Number   |      7      |
|    MsgSeqNo    |    667788     |   Number   |   0-65535   |
|    Interval    |     "30s"     |   String   |   5 chars   |

```json
{
  "PacketId": 7,
  "MsgSeqNo": 667788,
  "Interval": "30s"
}
```

## Gateway Heartbeat Publish Configuration Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       7       |   Number   |      7      |
|    MsgSeqNo    |    667788     |   Number   |   0-65535   |
|     Status     |   "Success"   |   String   |  10 chars   |

```json
{
  "PacketId": 7,
  "MsgSeqNo": 667788,
  "Status": "Success"
}
```

## Gateway Teaching Mode Packet

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       8       |   Number   |      8      |
|    MsgSeqNo    |    778899     |   Number   |   0-65535   |
|      Mode      |    "Teach"    |   String   |

10 chars |

```json
{
  "PacketId": 8,
  "MsgSeqNo": 778899,
  "Mode": "Teach"
}
```

## Gateway Teaching Mode Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       8       |   Number   |      8      |
|    MsgSeqNo    |    778899     |   Number   |   0-65535   |
|     Status     |   "Success"   |   String   |  10 chars   |

```json
{
  "PacketId": 8,
  "MsgSeqNo": 778899,
  "Status": "Success"
}
```

## Gateway Debug Info Packet

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       9       |   Number   |      9      |
|    MsgSeqNo    |    889900     |   Number   |   0-65535   |
|    InfoType    |    "Error"    |   String   |  10 chars   |

```json
{
  "PacketId": 9,
  "MsgSeqNo": 889900,
  "InfoType": "Error"
}
```

## Gateway Debug Info Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |       9       |   Number   |      9      |
|    MsgSeqNo    |    889900     |   Number   |   0-65535   |
|     Status     |   "Success"   |   String   |  10 chars   |

```json
{
  "PacketId": 9,
  "MsgSeqNo": 889900,
  "Status": "Success"
}
```

## Gateway General Ack

| Parameter Name | Example Value | Value Type | Value Range |
| :------------: | :-----------: | :--------: | :---------: |
|    PacketId    |      10       |   Number   |     10      |
|    MsgSeqNo    |    1000000    |   Number   |   0-65535   |
|     Status     |     "Ack"     |   String   |  10 chars   |

```json
{
  "PacketId": 10,
  "MsgSeqNo": 1000000,
  "Status": "Ack"
}
```
