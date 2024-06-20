const json_packet_enum = Object.freeze({
    /* GWY PACKETS */
    GWY_REG_PACKET: 0,
    GWY_CONF_PACKET : 1,
    GWY_UNREG_PACKET : 2,
    GWY_AC_CONTROL_PACKET : 3,
    GWY_MANUAL_AC_CONTROL_ACK_PACKET: 4,
    GWY_RECONF_PACKET: 5,
    GWY_TEMPERATURE_DATA_PACKET: 6,
    GWY_PUB_CONF_PACKET: 7,
    GWY_TEACHING_MODE_START_PACKET: 8,
    GWY_RESET_MQTT_PACKET: 99,

    /* NODE PACKETS */
    NODE_PROV_PACKET: 100,
    NODE_CONF_PACKET: 101,
    NODE_UNPROV_PACKET: 102,
    NODE_AC_CONTROL_PACKET: 103,
    NODE_MANUAL_AC_CONTROL_ACK_PACKET: 104,
    NODE_RECONF_PACKET: 105,
    NODE_TEMPERATURE_DATA_PACKET: 106,
    NODE_PUB_CONF_PACKET: 107,
    NODE_TEACHING_MODE_START_PACKET: 108,

    /* MISC PACKETS */
    UNKNOWN_PACKET: 9999
});

let randNum = Math.floor((Math.random() * 65535) + 1);

let Gwybasejson = {
    "JsonPacketID": msg.payload,
    "MsgSeqNo": randNum,
    "GwySerNo": global.get("GwySerNo"),
    "Location": global.get("Location")
};

let Nodebasejson = {
    "JsonPacketID": msg.payload,
    "MsgSeqNo": randNum,
    "GwySerNo": global.get("GwySerNo"),
    "NodeSerNo": global.get("NodeSerNo"),
    "ElementAddr": global.get("ElementAddr"),
    "Location": global.get("Location")
}

let json, mergedjson;
switch (msg.payload) {
    case json_packet_enum.NODE_AC_CONTROL_PACKET:
        json = {
            "Power": global.get("Power"),
            "Temperature": global.get("Temperature"),
            "FanSpeed": global.get("FanSpeed"),
            "Mode": global.get("Mode"),
            "SwingH": global.get("SwingH"),
            "SwingV": global.get("SwingV"),
            "Locking": global.get("Locking"),
            "OnTimer": global.get("OnTimer"),
            "OffTimer": global.get("OffTimer"),
            "TempLockUpLimit": global.get("TempLockUpLimit"),
            "TempLockLowLimit": global.get("TempLockLowLimit")
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.GWY_AC_CONTROL_PACKET:
        json = {
            "Power": global.get("Power"),
            "Temperature": global.get("Temperature"),
            "FanSpeed": global.get("FanSpeed"),
            "Mode": global.get("Mode"),
            "SwingH": global.get("SwingH"),
            "SwingV": global.get("SwingV"),
            "Locking": global.get("Locking"),
            "OnTimer": global.get("OnTimer"),
            "OffTimer": global.get("OffTimer"),
            "TempLockUpLimit": global.get("TempLockUpLimit"),
            "TempLockLowLimit": global.get("TempLockLowLimit")
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.NODE_PUB_CONF_PACKET:
        json = {
            "PublishPeriodSec": global.get("NodePublishPeriod")
        };
        msg = { payload: Object.assign({}, Nodebasejson, json) };
        break;

    case json_packet_enum.GWY_PUB_CONF_PACKET:
        json = {
            "PublishPeriodSec": global.get("GwyPublishPeriod")
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.NODE_PROV_PACKET:
        json = {
            "MacId": global.get("MacId")
        };
        msg = { payload: Object.assign({}, Nodebasejson, json) };
        break;

    case json_packet_enum.GWY_REG_PACKET:
    case json_packet_enum.GWY_UNREG_PACKET:
    case json_packet_enum.GWY_RECONF_PACKET:
    case json_packet_enum.GWY_TEACHING_MODE_START_PACKET:
        msg = { payload: Gwybasejson };
        break;

    case json_packet_enum.NODE_RECONF_PACKET:
    case json_packet_enum.NODE_UNPROV_PACKET:
    case json_packet_enum.NODE_TEACHING_MODE_START_PACKET:
        msg = { payload: Nodebasejson };
        break;
    
    default:
        Node.warn("Unknown JSON Packet ID");
}
return msg;