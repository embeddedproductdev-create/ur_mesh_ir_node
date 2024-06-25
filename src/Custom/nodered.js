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

let Gwybasejson = {
    "JsonPacketID": msg.payload,
    "MsgSeqNo": global.get("MsgSeqNo"),
    "GwySerNo": global.get("GwySerNo"),
};

let Nodebasejson = {
    "JsonPacketID": msg.payload,
    "MsgSeqNo": global.get("MsgSeqNo"),
    "GwySerNo": global.get("GwySerNo"),
    "NodeSerNo": global.get("NodeSerNo"),
    "ElementAddr": global.get("ElementAddr"),
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
            "Fan": global.get("FanSpeed"),
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

    case json_packet_enum.NODE_HEARTBEAT_PUB_CONF_PACKET:
        json = {
            "PublishPeriodSec": global.get("NodePublishPeriod")
        };
        msg = { payload: Object.assign({}, Nodebasejson, json) };
        break;

    case json_packet_enum.GWY_HEARTBEAT_PUB_CONF_PACKET:
        json = {
            "PublishPeriodSec": global.get("GwyPublishPeriod")
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.NODE_PROV_PACKET:
        json = {
            "Location": global.get("Location"),
            "MacId": global.get("MacId")
        };
        msg = { payload: Object.assign({}, Nodebasejson, json) };
        break;

    case json_packet_enum.GWY_REG_PACKET:
        json = {
            "Location": global.get("Location")
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.GWY_UNREG_PACKET:
        json = {
            "Location": global.get("Location")
        };
        msg = { payload: Object.assign({}, Gwybasejson, json) };
        break;

    case json_packet_enum.GWY_RECONF_PACKET:
        msg = { payload: Gwybasejson };
        break;
    case json_packet_enum.GWY_TEACHING_MODE_START_PACKET:
        msg = { payload: Gwybasejson };
        break;
    case json_packet_enum.GWY_DEBUG_INFO_PACKET:
        msg = { payload: Gwybasejson };
        break;

    case json_packet_enum.NODE_RECONF_PACKET:
    case json_packet_enum.NODE_UNPROV_PACKET:
        json = {
            "Location": global.get("Location")
        };
        msg = { payload: Object.assign({}, Nodebasejson, json) };
        break;

    case json_packet_enum.NODE_TEACHING_MODE_START_PACKET:
    case json_packet_enum.NODE_DEBUG_INFO_PACKET:
        msg = { payload: Nodebasejson };
        break;
    
    default:
        node.warn("Unknown JSON Packet ID");
        msg = { payload: Gwybasejson };
}
return msg;