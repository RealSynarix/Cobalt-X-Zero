import { u8, u16le, colorToBytes, bytesToColor, decodeString, encodeString } from "./hid.js";
export const FIELD={DEVICE_NAME:1,MANUFACTURER:2,SERIAL:3,FW_VERSION:4,MOD_SENSOR:16,MOD_BUTTONS:17,MOD_SCROLL:18,MOD_HYPRX:19,MOD_LEDS:20,MOD_SOF:21,DPI:32,POLL:33,LOD_MM:34,LOD_SQUAL:35,ANGLE_SNAP:36,ANGLE_STRENGTH:37,SURFACE:38,DB_PB3:48,DB_PB4:49,DB_PB5:50,DB_PB6:51,DB_PB7:52,CLICK_MODE:53,WHEEL_DIV:64,WHEEL_INV:65,WHEEL_SMOOTH:66,HYPRX_EN:80,HYPRX_LED:81,HYPRX_TARGET:82,HYPRX_STC:83,HYPRX_DIV:84,HYPRX_HOLD:85,LED_DEF:96,LED_MACRO:97,LED_BRI:98,LED_EFF:99,LED_SPEED:100,PB7_MODE:112,PB7_AT:113,PB7_MOUSE:114,PB7_KEY:115,PB7_COMBO:116,PB7_MEDIA:117};
export const FIELD_META={
[FIELD.DEVICE_NAME]:{name:"device_name",type:"string",ro:true,maxLen:32},
[FIELD.MANUFACTURER]:{name:"manufacturer",type:"string",ro:true,maxLen:32},
[FIELD.SERIAL]:{name:"serial",type:"string",ro:true,maxLen:16},
[FIELD.FW_VERSION]:{name:"fw_version",type:"string",ro:true,maxLen:16},
[FIELD.MOD_SENSOR]:{name:"module_sensor",type:"bool"},
[FIELD.MOD_BUTTONS]:{name:"module_buttons",type:"bool"},
[FIELD.MOD_SCROLL]:{name:"module_scroll",type:"bool"},
[FIELD.MOD_HYPRX]:{name:"module_hyprx",type:"bool"},
[FIELD.MOD_LEDS]:{name:"module_leds",type:"bool"},
[FIELD.MOD_SOF]:{name:"module_sof",type:"bool"},
[FIELD.DPI]:{name:"dpi",type:"u16",min:100,max:32000},
[FIELD.POLL]:{name:"polling_rate",type:"u16",min:125,max:8000},
[FIELD.LOD_MM]:{name:"lod_mm",type:"u8",min:1,max:3},
[FIELD.LOD_SQUAL]:{name:"lod_squal",type:"u8",min:10,max:200},
[FIELD.ANGLE_SNAP]:{name:"angle_snap",type:"bool"},
[FIELD.ANGLE_STRENGTH]:{name:"angle_strength",type:"u8",min:0,max:100},
[FIELD.SURFACE]:{name:"surface",type:"u8",min:0,max:3},
[FIELD.DB_PB3]:{name:"debounce_pb3",type:"u8",min:0,max:20},
[FIELD.DB_PB4]:{name:"debounce_pb4",type:"u8",min:0,max:20},
[FIELD.DB_PB5]:{name:"debounce_pb5",type:"u8",min:0,max:20},
[FIELD.DB_PB6]:{name:"debounce_pb6",type:"u8",min:0,max:20},
[FIELD.DB_PB7]:{name:"debounce_pb7",type:"u8",min:0,max:20},
[FIELD.CLICK_MODE]:{name:"click_mode",type:"u8",min:0,max:2},
[FIELD.WHEEL_DIV]:{name:"wheel_div",type:"u8",min:1,max:8},
[FIELD.WHEEL_INV]:{name:"wheel_invert",type:"bool"},
[FIELD.WHEEL_SMOOTH]:{name:"wheel_smooth",type:"bool"},
[FIELD.HYPRX_EN]:{name:"hyprx_enable",type:"bool"},
[FIELD.HYPRX_LED]:{name:"hyprx_change_led",type:"bool"},
[FIELD.HYPRX_TARGET]:{name:"hyprx_target",type:"u8",min:0,max:1},
[FIELD.HYPRX_STC]:{name:"hyprx_scroll_to_click",type:"u8",min:1,max:24},
[FIELD.HYPRX_DIV]:{name:"hyprx_div",type:"u8",min:1,max:4},
[FIELD.HYPRX_HOLD]:{name:"hyprx_hold",type:"u16",min:0,max:500},
[FIELD.LED_DEF]:{name:"led_default",type:"color"},
[FIELD.LED_MACRO]:{name:"led_macro",type:"color"},
[FIELD.LED_BRI]:{name:"led_brightness",type:"u8",min:0,max:255},
[FIELD.LED_EFF]:{name:"led_effect",type:"u8",min:0,max:4},
[FIELD.LED_SPEED]:{name:"led_speed",type:"u8",min:0,max:255},
[FIELD.PB7_MODE]:{name:"pb7_mode",type:"u8",min:0,max:2},
[FIELD.PB7_AT]:{name:"pb7_action",type:"u8",min:0,max:3},
[FIELD.PB7_MOUSE]:{name:"pb7_mouse",type:"u8",min:0,max:7},
[FIELD.PB7_KEY]:{name:"pb7_key",type:"u8",min:0,max:255},
[FIELD.PB7_COMBO]:{name:"pb7_combo",type:"combo",maxLen:8},
[FIELD.PB7_MEDIA]:{name:"pb7_media",type:"u8",min:0,max:12},
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
let n=Number(value);
if(!Number.isFinite(n))return {ok:false,err:"number expected"};
if(!Number.isInteger(n))return {ok:false,err:"integer expected"};
if(n<m.min||n>m.max)return {ok:false,err:`out of range ${m.min}-${m.max}`};
return {ok:true,value:n};
}
if(m.type==="color"){
let s=String(value).trim();
if(!/^#[0-9a-fA-F]{6}$/.test(s))return {ok:false,err:"color #RRGGBB"};
return {ok:true,value:s.toLowerCase()};
}
if(m.type==="string"){
let s=String(value);
if(s.length>m.maxLen)return {ok:false,err:`max ${m.maxLen}`};
return {ok:true,value:s};
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
const v=validateField(fid,value);
if(!v.ok&&!meta.ro)throw new Error(v.err);
let val=v.ok?v.value:value;
if(meta.ro)val=value;
if(meta.type==="bool")return u8(val?1:0);
if(meta.type==="u8")return u8(clamp(val,meta.min,meta.max));
if(meta.type==="u16")return u16le(clamp(val,meta.min,meta.max));
if(meta.type==="color")return colorToBytes(val);
if(meta.type==="string")return encodeString(String(val),meta.maxLen||32);
if(meta.type==="combo"){
if(String(val).trim()==="")return new Uint8Array(0);
const parts=String(val).split(',').map(s=>parseInt(s.trim(),10)).filter(n=>!isNaN(n)&&n>=0&&n<=255);
return new Uint8Array(parts.slice(0,8));
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
const els=document.querySelectorAll(`[data-field="${fid}"]`);
els.forEach(el=>{if(el.type==="checkbox")el.checked=!!val;else el.value=val??"";});
if(fid===FIELD.LED_BRI){const v=document.getElementById("f-led_bri-v");if(v)v.textContent=String(val);}
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
const payload=payloadForField(fid,res.value);
out.push({fieldId:fid,payload,raw:res.value});
});
if(errs.length)throw new Error(errs.join("\n"));
return out;
}
export function loadConfigToUI(cfg){
for(const [k,v] of Object.entries(cfg)){
const fid=parseInt(k,10);
try{const p=payloadForField(fid,v);applyFieldToUI(fid,p);}catch{}
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
