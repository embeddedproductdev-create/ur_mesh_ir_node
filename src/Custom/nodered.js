const json_packet_enum = Object.freeze({
    /* GWY PACKETS */
    GWY_REG_PACKET: 0,
    GWY_CONF_ACK : 1,
    GWY_UNREG_PACKET : 2,
    GWY_AC_CONTROL_PACKET : 3,
    GWY_MANUAL_AC_CONTROL_ACK: 4,
    GWY_RECONF_PACKET: 5,
    GWY_HEARTBEAT_ACK: 6,
    GWY_HEARTBEAT_PUB_CONF_PACKET: 7,
    GWY_TEACHING_MODE_START_PACKET: 8,
    GWY_TEACHING_MODE_END_ACK: 9,
    GWY_DEBUG_INFO_PACKET: 10,
    GWY_RESET_MQTT_PACKET: 99,

    /* NODE PACKETS */
    NODE_PROV_PACKET: 100,
    NODE_CONF_ACK: 101,
    NODE_UNPROV_PACKET: 102,
    NODE_AC_CONTROL_PACKET: 103,
    NODE_MANUAL_AC_CONTROL_ACK_PACKET: 104,
    NODE_RECONF_PACKET: 105,
    NODE_HEARTBEAT_ACK: 106,
    NODE_HEARTBEAT_PUB_CONF_PACKET: 107,
    NODE_TEACHING_MODE_START_PACKET: 108,
    NODE_TEACHING_MODE_END_ACK: 109,
    NODE_DEBUG_INFO_PACKET: 110,

    /* MISC PACKETS */
    UNKNOWN_PACKET: 9999
});

let randNum = Math.floor((Math.random() * 65535) + 1);

let Gwybasejson = {
    "JsonPacketID": msg.payload,
    "MsgSeqNo": randNum,
    "GwySerNo": flow.get("GwySerNo"),
};

let Nodebasejson = {
    "JsonPacketID": msg.payload,
    "MsgSeqNo": randNum,
    "GwySerNo": flow.get("GwySerNo"),
    "NodeSerNo": flow.get("NodeSerNo"),
    "ElementAddr": flow.get("ElementAddr"),
}

let json, mergedjson;
switch (msg.payload) {
    case json_packet_enum.NODE_AC_CONTROL_PACKET:
        json = {
            "Power": flow.get("Power"),
            "Temperature": flow.get("Temperature"),
            "FanSpeed": flow.get("FanSpeed"),
            "Mode": flow.get("Mode"),
            "SwingH": flow.get("SwingH"),
            "SwingV": flow.get("SwingV"),
            "Locking": flow.get("Locking"),
            "OnTimer": flow.get("OnTimer"),
            "OffTimer": flow.get("OffTimer"),
            "TempLockUpLimit": flow.get("TempLockUpLimit"),
            "TempLockLowLimit": flow.get("TempLockLowLimit")
        };
        msg = { payload: Object.assign({}, Nodebasejson, json) };
        break;

    case json_packet_enum.GWY_AC_CONTROL_PACKET:
        json = {
            "Power": flow.get("Power"),
            "Temperature": flow.get("Temperature"),
            "FanSpeed": flow.get("FanSpeed"),
            "Mode": flow.get("Mode"),
            "SwingH": flow.get("SwingH"),
            "SwingV": flow.get("SwingV"),
            "Locking": flow.get("Locking"),
            "OnTimer": flow.get("OnTimer"),
            "OffTimer": flow.get("OffTimer"),
            "TempLockUpLimit": flow.get("TempLockUpLimit"),
            "TempLockLowLimit": flow.get("TempLockLowLimit")
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.NODE_HEARTBEAT_PUB_CONF_PACKET:
        json = {
            "PublishPeriodSec": flow.get("NodePublishPeriod")
        };
        msg = { payload: Object.assign({}, Nodebasejson, json) };
        break;

    case json_packet_enum.GWY_HEARTBEAT_PUB_CONF_PACKET:
        json = {
            "PublishPeriodSec": flow.get("GwyPublishPeriod")
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.NODE_PROV_PACKET:
        json = {
            "JsonPacketID": msg.payload,
            "MsgSeqNo": randNum,
            "GwySerNo": flow.get("GwySerNo"),
            "NodeSerNo": flow.get("NodeSerNo"),
            "Location": flow.get("Location"),
            "MacId": flow.get("MacId")
        };
        msg = {payload: json};
        break;

    case json_packet_enum.GWY_REG_PACKET:
        json = {
            "Location": flow.get("Location")
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.GWY_UNREG_PACKET:
        json = {
            "Location": flow.get("Location")
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.GWY_RECONF_PACKET:
    case json_packet_enum.GWY_TEACHING_MODE_START_PACKET:
        msg = { payload: Gwybasejson };
        break;
    
    case json_packet_enum.NODE_DEBUG_INFO_PACKET:
        json = {
            "ResetDevice": 0,
            "Logging": 0
        };
        msg = { payload: Object.assign({}, Nodebasejson, json) };
        break;

    case json_packet_enum.GWY_DEBUG_INFO_PACKET:
        json = {
            "ResetDevice": 0,
            "Logging":0
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.NODE_RECONF_PACKET:
    case json_packet_enum.NODE_UNPROV_PACKET:
        json = {
            "ElementAddr":flow.get("ElementAddr"),
            "Location": flow.get("Location")
        };
        msg = { payload: Object.assign({}, Nodebasejson, json) };
        break;

    case json_packet_enum.NODE_TEACHING_MODE_START_PACKET:
        msg = { payload: Nodebasejson };
        break;
}
return msg;