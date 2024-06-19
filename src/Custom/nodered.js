const json_packet_enum = Object.freeze({
    /* GWY PACKETS */
	GWY_REG_PACKET,
	GWY_CONF_PACKET,
	GWY_UNREG_PACKET,
	GWY_AC_CONTROL_PACKET,
	GWY_MANUAL_AC_CONTROL_ACK_PACKET,
	GWY_RECONF_PACKET,
	GWY_TEMPERATURE_DATA_PACKET,
	GWY_PUB_CONF_PACKET,
	RESET_MQTT,
	GWY_TEACHING_MODE_START_PACKET,

    /* NODE PACKETS */
	NODE_PROV_PACKET: 100,
	NODE_CONF_PACKET,
	NODE_UNPROV_PACKET,
	NODE_AC_CONTROL_PACKET,
	NODE_MANUAL_AC_CONTROL_ACK_PACKET,
	NODE_RECONF_PACKET,
	NODE_TEMPERATURE_DATA_PACKET,
	NODE_PUB_CONF_PACKET,
	NODE_TEACHING_MODE_START_PACKET,

    /* MISC PACKETS */
	UNKNOWN_PACKET: 99
});

let randNum = Math.floor((Math.random() * 65535) + 1);

let Gwybasejson = {
    "MsgSeqNo" : randNum,
    "GwySerNo" : global.get("GwySerNo"),
};
let json, mergedjson;
switch(msg.payload)
{
    case json_packet_enum.GWY_REG_PACKET:
        json = {
            "JsonPacketID" : 0,
            "Location" : "1st Floor"
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case json_packet_enum.GWY_UNREG_PACKET:
        json = {
            "JsonPacketID" : 2,
            "Location" : "1st Floor"
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case json_packet_enum.GWY_AC_CONTROL_PACKET:
        json = {
            "JsonPacketID" : 3,
            "Power" : global.get("Power"),
            "Temperature" : global.get("Temperature"),
            "FanSpeed" : global.get("FanSpeed"),
            "Mode" : global.get("Mode"),
            "SwingH" : global.get("SwingH"),
            "SwingV" : global.get("SwingV"),
            "Locking" : global.get("Locking"),
            "OnTimer" : global.get("OnTimer"),
            "OffTimer" : global.get("OffTimer"),
            "TempLockUpLimit" : global.get("TempLockUpLimit"),
            "TempLockLowLimit" : global.get("TempLockLowLimit")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case json_packet_enum.GWY_RECONF_PACKET:
        json = {
            "JsonPacketId" : 5
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case json_packet_enum.GWY_PUB_CONF_PACKET:
        json = {
            "JsonPacketId" : 7,
            "PublishPeriodSec" : global.get("GwyPublishPeriod")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case json_packet_enum.NODE_PROV_PACKET:
        json = {
            "JsonPacketId" : 100,
            "Location" : "1st Floor",
            "NodeSerNo" : global.get("NodeSerNo"),
            "MacId" : global.get("MacId")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case json_packet_enum.NODE_UNPROV_PACKET:
        json = {
            "JsonPacketId" : 102,
            "ElementAddr" : global.get("ElementAddr"),
            "NodeSerNo" : global.get("NodeSerNo")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case json_packet_enum.NODE_AC_CONTROL_PACKET:
        json = {
            "NodeSerNo" : global.get("NodeSerNo"),
            "ElementAddr" : global.get("ElementAddr"),
            "JsonPacketID" : 103,
            "Power" : global.get("Power"),
            "Temperature" : global.get("Temperature"),
            "FanSpeed" : global.get("FanSpeed"),
            "Mode" : global.get("Mode"),
            "SwingH" : global.get("SwingH"),
            "SwingV" : global.get("SwingV"),
            "Locking" : global.get("Locking"),
            "OnTimer" : global.get("OnTimer"),
            "OffTimer" : global.get("OffTimer"),
            "TempLockUpLimit" : global.get("TempLockUpLimit"),
            "TempLockLowLimit" : global.get("TempLockLowLimit")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case json_packet_enum.NODE_RECONF_PACKET:
        json = {
            "JsonPacketId" : 105,
            "ElementAddr" : global.get("ElementAddr"),
            "NodeSerNo" : global.get("NodeSerNo")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
        
    case json_packet_enum.NODE_PUB_CONF_PACKET:
        json = {
            "JsonPacketId" : 107,
            "ElementAddr" : global.get("ElementAddr"),
            "NodeSerNo" : global.get("NodeSerNo"),
            "PublishPeriodSec" : global.get("NodePublishPeriod")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
    
    case 
}
return msg;