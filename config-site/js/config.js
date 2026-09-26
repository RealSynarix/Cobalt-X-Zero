import { u8, u16le, colorToBytes, bytesToColor, decodeString, encodeString } from "./hid.js";
export const FIELD={DEVICE_NAME:1,MANUFACTURER:2,SERIAL:3,FW_VERSION:4,MOD_SENSOR:16,MOD_BUTTONS:17,MOD_SCROLL:18,MOD_HYPRX:19,MOD_LEDS:20,MOD_SOF:21,DPI:32,POLL:33,LOD_MM:34,LOD_SQUAL:35,ANGLE_SNAP:36,ANGLE_STRENGTH:37,SURFACE:38,DB_PB3:48,DB_PB4:49,DB_PB5:50,DB_PB6:51,DB_PB7:52,CLICK_MODE:53,WHEEL_DIV:64,WHEEL_INV:65,WHEEL_SMOOTH:66,HYPRX_EN:80,HYPRX_LED:81,HYPRX_TARGET:82,HYPRX_STC:83,HYPRX_DIV:84,HYPRX_HOLD:85,LED_DEF:96,LED_MACRO:97,LED_BRI:98,LED_EFF:99,LED_SPEED:100,PB7_MODE:112,PB7_AT:113,PB7_MOUSE:114,PB7_KEY:115,PB7_COMBO:116,PB7_MEDIA:117};
export const FIELD_META={
[1]:{name:"device_name",type:"string",ro:true,maxLen:32},
[2]:{name:"manufacturer",type:"string",ro:true,maxLen:32},
[3]:{name:"serial",type:"string",ro:true,maxLen:16},
[4]:{name:"fw_version",type:"string",ro:true,maxLen:16},
[16]:{name:"module_sensor",type:"bool"},
[17]:{name:"module_buttons",type:"bool"},
[18]:{name:"module_scroll",type:"bool"},
[19]:{name:"module_hyprx",type:"bool"},
[20]:{name:"module_leds",type:"bool"},
[21]:{name:"module_sof",type:"bool"},
[32]:{name:"dpi",type:"u16",min:100,max:32000},
[33]:{name:"polling_rate",type:"u16",min:125,max:8000},
[34]:{name:"lod_mm",type:"u8",min:1,max:3},
[35]:{name:"lod_squal",type:"u8",min:10,max:200},
[36]:{name:"angle_snap",type:"bool"},
[37]:{name:"angle_strength",type:"u8",min:0,max:100},
[38]:{name:"surface",type:"u8",min:0,max:3},
[48]:{name:"debounce_pb3",type:"u8",min:0,max:20},
[49]:{name:"debounce_pb4",type:"u8",min:0,max:20},
[50]:{name:"debounce_pb5",type:"u8",min:0,max:20},
[51]:{name:"debounce_pb6",type:"u8",min:0,max:20},
[52]:{name:"debounce_pb7",type:"u8",min:0,max:20},
[53]:{name:"click_mode",type:"u8",min:0,max:2},
[64]:{name:"wheel_div",type:"u8",min:1,max:8},
[65]:{name:"wheel_invert",type:"bool"},
[66]:{name:"wheel_smooth",type:"bool"},
[80]:{name:"hyprx_enable",type:"bool"},
[81]:{name:"hyprx_change_led",type:"bool"},
[82]:{name:"hyprx_target",type:"u8",min:0,max:1},
[83]:{name:"hyprx_scroll_to_click",type:"u8",min:1,max:24},
[84]:{name:"hyprx_div",type:"u8",min:1,max:4},
[85]:{name:"hyprx_hold",type:"u16",min:0,max:500},
[96]:{name:"led_default",type:"color"},
[97]:{name:"led_macro",type:"color"},
[98]:{name:"led_brightness",type:"u8",min:0,max:255},
[99]:{name:"led_effect",type:"u8",min:0,max:4},
[100]:{name:"led_speed",type:"u8",min:0,max:255},
[112]:{name:"pb7_mode",type:"u8",min:0,max:2},
[113]:{name:"pb7_action",type:"u8",min:0,max:3},
[114]:{name:"pb7_mouse",type:"u8",min:0,max:7},
[115]:{name:"pb7_key",type:"u8",min:0,max:255},
[116]:{name:"pb7_combo",type:"combo",maxLen:8},
[117]:{name:"pb7_media",type:"u8",min:0,max:12},
};
export let currentConfig={};
export function clamp(v,min,max){v=Number(v);if(Number.isNaN(v))return min;return Math.min(max,Math.max(min,v));}
export function validateField(fid,value){
const m=FIELD_META[fid];
if(!m)return {ok:false,err:"unknown field"};
if(m.ro)return {ok:false,err:"read only"};
if(m.type==="bool"){
if(typeof value==="string"){if(value==="true")value=true;else if(value==="false")value=false;else return {ok:false,err:"bool expected"};}
if(typeof value!=="boolean")return {ok:false,err:"bool expected"};
return {ok:true,value:!!value};
}
if(m.type==="u8"||m.type==="u16"){
let n=Number(value);if(!Number.isFinite(n))return {ok:false,err:"number"};
if(!Number.isInteger(n))return {ok:false,err:"integer"};
if(n<m.min||n>m.max)return {ok:false,err:`range ${m.min}-${m.max}`};
return {ok:true,value:n};
}
if(m.type==="color"){
let s=String(value).trim();
if(!/^#[0-9a-fA-F]{6}$/.test(s))return {ok:false,err:"color #RRGGBB"};
return {ok:true,value:s.toLowerCase()};
}
if(m.type==="combo"){
let str=String(value).trim();
if(str==="")return {ok:true,value:""};
let parts=str.split(',').map(x=>x.trim()).filter(x=>x!=="");
if(parts.length>m.maxLen)return {ok:false,err:`max ${m.maxLen}`};
for(let p of parts){let n=Number(p);if(!Number.isInteger(n)||n<0||n>255)return {ok:false,err:"0-255"};}
return {ok:true,value:parts.join(',')};
}
return {ok:false,err:"unknown"};
}
export function payloadForField(fid,value){
const meta=FIELD_META[fid];
if(!meta)return new Uint8Array(0);
if(meta.ro)return encodeString(String(value),meta.maxLen||32);
const v=validateField(fid,value);
if(!v.ok)throw new Error(v.err);
let val=v.value;
if(meta.type==="bool")return u8(val?1:0);
if(meta.type==="u8")return u8(clamp(val,meta.min,meta.max));
if(meta.type==="u16")return u16le(clamp(val,meta.min,meta.max));
if(meta.type==="color")return colorToBytes(val);
if(meta.type==="combo"){
if(String(val).trim()==="")return new Uint8Array(0);
return new Uint8Array(String(val).split(',').map(s=>parseInt(s.trim(),10)).filter(n=>!isNaN(n)));
}
return new Uint8Array(0);
}
export function valueFromPayload(fid,payload){
const m=FIELD_META[fid];
if(!m||!payload)return null;
if(m.type==="bool")return payload[0]?true:false;
if(m.type==="u8")return payload[0];
if(m.type==="u16")return payload[0]|(payload[1]<<8);
if(m.type==="color")return bytesToColor(payload);
if(m.type==="string")return decodeString(payload);
if(m.type==="combo")return Array.from(payload).join(',');
return payload;
}
export function applyFieldToUI(fid,payload){
const val=valueFromPayload(fid,payload);
currentConfig[fid]=val;
document.querySelectorAll(`[data-field="${fid}"]`).forEach(el=>{if(el.type==="checkbox")el.checked=!!val;else el.value=val??"";});
}
export function gatherAllFields(){
const out=[];let errs=[];
document.querySelectorAll("[data-field]").forEach(el=>{
const fid=parseInt(el.getAttribute("data-field"),10);
const meta=FIELD_META[fid];
if(!meta||meta.ro)return;
let raw=el.type==="checkbox"?el.checked:el.value;
const res=validateField(fid,raw);
if(!res.ok){errs.push(`${meta.name}: ${res.err}`);return;}
out.push({fieldId:fid,payload:payloadForField(fid,res.value)});
});
if(errs.length)throw new Error(errs.join("\n"));
return out;
}
export function loadConfigToUI(cfg){
for(const [k,v] of Object.entries(cfg)){
const fid=parseInt(k,10);
try{applyFieldToUI(fid,payloadForField(fid,v));}catch{}
}
currentConfig={...cfg};
}
export function getCurrentConfigObject(){
const obj={};
for(const fid of Object.keys(FIELD_META)){
const id=parseInt(fid,10);
const el=document.querySelector(`[data-field="${id}"]`);
if(!el)continue;
obj[id]=el.type==="checkbox"?el.checked:el.value;
}
return obj;
}
