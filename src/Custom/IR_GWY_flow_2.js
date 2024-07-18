[
    {
        "id": "c16e1b90dc62d2e7",
        "type": "function",
        "z": "9c54f3c8c089fa45",
        "g": "d43bb397402bc867",
        "name": "Json Injector",
        "func": "const json_packet_enum = Object.freeze({\n    /* GWY PACKETS */\n    GWY_REG_PACKET: 0,\n    GWY_CONF_ACK : 1,\n    GWY_UNREG_PACKET : 2,\n    GWY_AC_CONTROL_PACKET : 3,\n    GWY_MANUAL_AC_CONTROL_ACK: 4,\n    GWY_RECONF_PACKET: 5,\n    GWY_HEARTBEAT_ACK: 6,\n    GWY_HEARTBEAT_PUB_CONF_PACKET: 7,\n    GWY_TEACHING_MODE_START_PACKET: 8,\n    GWY_TEACHING_MODE_END_ACK: 9,\n    GWY_DEBUG_INFO_PACKET: 10,\n    GWY_RESET_MQTT_PACKET: 99,\n\n    /* NODE PACKETS */\n    NODE_PROV_PACKET: 100,\n    NODE_CONF_ACK: 101,\n    NODE_UNPROV_PACKET: 102,\n    NODE_AC_CONTROL_PACKET: 103,\n    NODE_MANUAL_AC_CONTROL_ACK_PACKET: 104,\n    NODE_RECONF_PACKET: 105,\n    NODE_HEARTBEAT_ACK: 106,\n    NODE_HEARTBEAT_PUB_CONF_PACKET: 107,\n    NODE_TEACHING_MODE_START_PACKET: 108,\n    NODE_TEACHING_MODE_END_ACK: 109,\n    NODE_DEBUG_INFO_PACKET: 110,\n\n    /* MISC PACKETS */\n    UNKNOWN_PACKET: 9999\n});\n\nlet randNum = Math.floor((Math.random() * 65535) + 1);\n\nlet Gwybasejson = {\n    \"JsonPacketID\": msg.payload,\n    \"MsgSeqNo\": randNum,\n    \"GwySerNo\": flow.get(\"GwySerNo\"),\n};\n\nlet Nodebasejson = {\n    \"JsonPacketID\": msg.payload,\n    \"MsgSeqNo\": randNum,\n    \"GwySerNo\": flow.get(\"GwySerNo\"),\n    \"NodeSerNo\": flow.get(\"NodeSerNo\"),\n    \"ElementAddr\": flow.get(\"ElementAddr\"),\n}\n\nlet json, mergedjson;\nswitch (msg.payload) {\n    case json_packet_enum.NODE_AC_CONTROL_PACKET:\n        json = {\n            \"Power\": flow.get(\"Power\"),\n            \"Temperature\": flow.get(\"Temperature\"),\n            \"FanSpeed\": flow.get(\"FanSpeed\"),\n            \"Mode\": flow.get(\"Mode\"),\n            \"SwingH\": flow.get(\"SwingH\"),\n            \"SwingV\": flow.get(\"SwingV\"),\n            \"Locking\": flow.get(\"Locking\"),\n            \"OnTimer\": flow.get(\"OnTimer\"),\n            \"OffTimer\": flow.get(\"OffTimer\"),\n            \"TempLockUpLimit\": flow.get(\"TempLockUpLimit\"),\n            \"TempLockLowLimit\": flow.get(\"TempLockLowLimit\")\n        };\n        msg = { payload: Object.assign({}, Nodebasejson, json) };\n        break;\n\n    case json_packet_enum.GWY_AC_CONTROL_PACKET:\n        json = {\n            \"Power\": flow.get(\"Power\"),\n            \"Temperature\": flow.get(\"Temperature\"),\n            \"FanSpeed\": flow.get(\"FanSpeed\"),\n            \"Mode\": flow.get(\"Mode\"),\n            \"SwingH\": flow.get(\"SwingH\"),\n            \"SwingV\": flow.get(\"SwingV\"),\n            \"Locking\": flow.get(\"Locking\"),\n            \"OnTimer\": flow.get(\"OnTimer\"),\n            \"OffTimer\": flow.get(\"OffTimer\"),\n            \"TempLockUpLimit\": flow.get(\"TempLockUpLimit\"),\n            \"TempLockLowLimit\": flow.get(\"TempLockLowLimit\")\n        };\n        msg = { payload: Object.assign({}, Gwybasejson, json) };\n        break;\n\n    case json_packet_enum.NODE_HEARTBEAT_PUB_CONF_PACKET:\n        json = {\n            \"PublishPeriodSec\": flow.get(\"NodePublishPeriod\")\n        };\n        msg = { payload: Object.assign({}, Nodebasejson, json) };\n        break;\n\n    case json_packet_enum.GWY_HEARTBEAT_PUB_CONF_PACKET:\n        json = {\n            \"PublishPeriodSec\": flow.get(\"GwyPublishPeriod\")\n        };\n        msg = { payload: Object.assign({}, Gwybasejson, json) };\n        break;\n\n    case json_packet_enum.NODE_PROV_PACKET:\n        json = {\n            \"JsonPacketID\": msg.payload,\n            \"MsgSeqNo\": randNum,\n            \"GwySerNo\": flow.get(\"GwySerNo\"),\n            \"NodeSerNo\": flow.get(\"NodeSerNo\"),\n            \"Location\": flow.get(\"Location\"),\n            \"MacId\": flow.get(\"MacId\")\n        };\n        msg = {payload: json};\n        break;\n\n    case json_packet_enum.GWY_REG_PACKET:\n        json = {\n            \"Location\": flow.get(\"Location\")\n        };\n        msg = { payload: Object.assign({}, Gwybasejson, json) };\n        break;\n\n    case json_packet_enum.GWY_UNREG_PACKET:\n        json = {\n            \"Location\": flow.get(\"Location\")\n        };\n        msg = { payload: Object.assign({}, Gwybasejson, json) };\n        break;\n\n    case json_packet_enum.GWY_RECONF_PACKET:\n    case json_packet_enum.GWY_TEACHING_MODE_START_PACKET:\n        msg = { payload: Gwybasejson };\n        break;\n    \n    case json_packet_enum.NODE_DEBUG_INFO_PACKET:\n        json = {\n            \"ResetDevice\": 0,\n            \"Logging\": 0\n        };\n        msg = { payload: Object.assign({}, Nodebasejson, json) };\n        break;\n\n    case json_packet_enum.GWY_DEBUG_INFO_PACKET:\n        json = {\n            \"ResetDevice\": 0,\n            \"Logging\":0\n        };\n        msg = { payload: Object.assign({}, Gwybasejson, json) };\n        break;\n\n\n    case json_packet_enum.NODE_UNPROV_PACKET:\n        json = {\n            \"Location\": flow.get(\"Location\")\n        };\n        msg = { payload: Object.assign({}, Nodebasejson, json) };\n        break;\n    \n    case json_packet_enum.NODE_RECONF_PACKET:\n    case json_packet_enum.NODE_TEACHING_MODE_START_PACKET:\n        msg = { payload: Nodebasejson };\n        break;\n}\nreturn msg;",
        "outputs": 1,
        "timeout": "",
        "noerr": 0,
        "initialize": "",
        "finalize": "",
        "libs": [],
        "x": 305,
        "y": 320,
        "wires": [
            [
                "c3ad127614d97a3d",
                "e38002df5550c456"
            ]
        ],
        "inputLabels": [
            "Json Packet ID"
        ],
        "outputLabels": [
            "JSON"
        ],
        "l": false,
        "info": "Creates Dynamic JSON as per User Input Values"
    }
]