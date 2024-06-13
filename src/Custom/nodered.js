const GWY_REGISTER=0; 
const GWY_UNREGISTER=2;
const GWY_CONTROL=3;
const GWY_RECONF=5;
const GWY_PUBCONF=7;

const NODE_PROV=100;
const NODE_UNPROV=102;
const NODE_CONTROL=103;
const NODE_RECONF=105;
const NODE_PUBCONF=107;

let randNum = Math.floor((Math.random() * 65535) + 1);

let Gwybasejson = {
    "MsgSeqNo" : randNum,
    "GwySerNo" : global.get("GwySerNo"),
};
let json, mergedjson;
switch(msg.payload)
{
    case GWY_REGISTER:
        json = {
            "JsonPacketID" : 0,
            "Location" : "1st Floor"
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case GWY_UNREGISTER:
        json = {
            "JsonPacketID" : 2,
            "Location" : "1st Floor"
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case GWY_CONTROL:
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
            "TempUpLockLimit" : global.get("TempUpLockLimit"),
            "TempLowLockLimit" : global.get("TempLowLockLimit")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case GWY_RECONF:
        json = {
            "JsonPacketId" : 5
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case GWY_PUBCONF:
        json = {
            "JsonPacketId" : 7,
            "PublishPeriodSec" : global.get("GwyPublishPeriod")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case NODE_PROV:
        json = {
            "JsonPacketId" : 100,
            "Location" : "1st Floor",
            "NodeSerNo" : global.get("NodeSerNo"),
            "MacId" : global.get("MacId")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case NODE_UNPROV:
        json = {
            "JsonPacketId" : 102,
            "ElementAddr" : global.get("ElementAddr"),
            "NodeSerNo" : global.get("NodeSerNo")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case NODE_CONTROL:
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
            "TempUpLockLimit" : global.get("TempUpLockLimit"),
            "TempLowLockLimit" : global.get("TempLowLockLimit")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
    case NODE_RECONF:
        json = {
            "JsonPacketId" : 105,
            "ElementAddr" : global.get("ElementAddr"),
            "NodeSerNo" : global.get("NodeSerNo")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
        
        
    case NODE_PUBCONF:
        json = {
            "JsonPacketId" : 107,
            "ElementAddr" : global.get("ElementAddr"),
            "NodeSerNo" : global.get("NodeSerNo"),
            "PublishPeriodSec" : global.get("NodePublishPeriod")
        };
        msg = {payload : Object.assign({}, Gwybasejson, json)};
        break;
}
return msg;