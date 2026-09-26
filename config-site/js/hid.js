export let device=null;
export let connected=false;
const REPORT_ID=2;
const PACKET_SIZE=64;
const CMD_READ=0x01;
const CMD_WRITE=0x02;
const CMD_GET_ALL=0x03;
const CMD_RESET=0x04;
const CMD_PING=0x05;
export function makePacket(cmd,fieldId,payload){
const buf=new Uint8Array(PACKET_SIZE);
const p=payload instanceof Uint8Array?payload:new Uint8Array(payload||[]);
let len=p.length;
if(len>60)len=60;
buf[0]=cmd&0xff;
buf[1]=fieldId&0xff;
buf[2]=len&0xff;
buf[3]=(len>>8)&0xff;
buf.set(p.subarray(0,len),4);
return buf;
}
export function encodeString(str,maxLen){
const enc=new TextEncoder();
const b=enc.encode(str);
if(maxLen&&b.length>maxLen)return b.slice(0,maxLen);
return b;
}
export function decodeString(u8){
try{
const dec=new TextDecoder();
let end=u8.length;
for(let i=0;i<u8.length;i++){if(u8[i]===0){end=i;break;}}
return dec.decode(u8.slice(0,end));
}catch{return "";}
}
export async function openDevice(d){
if(!d)return false;
if(!d.opened)await d.open();
device=d;
connected=true;
return true;
}
export function parseResponse(raw){
const data=raw instanceof Uint8Array?raw:new Uint8Array(raw);
let off=0;
if(data[0]===REPORT_ID)off=1;
if(data.length<5+off)return {cmd:0,fieldId:0,len:0,status:255,payload:new Uint8Array(0),raw:data};
const cmd=data[off];
const fieldId=data[off+1];
const len=data[off+2]|(data[off+3]<<8);
const status=data[off+4];
const payload=data.slice(off+5,off+5+len);
return {cmd,fieldId,len,status,payload,raw:data};
}
export async function sendPacketAndReadResponse(packet){
if(!device||!device.opened)throw new Error("No device");
const toSend=packet.length===64?packet:(()=>{const b=new Uint8Array(64);b.set(packet);return b;})();
let sent=false;
try{await device.sendReport(REPORT_ID,toSend);sent=true;}catch{}
if(!sent){try{await device.sendReport(0,toSend);sent=true;}catch{}}
if(!sent){try{await device.sendFeatureReport(REPORT_ID,toSend);sent=true;}catch{}}
if(!sent)throw new Error("send failed");
await new Promise(r=>setTimeout(r,90));
let resp=null;
try{
const fr=await device.receiveFeatureReport(REPORT_ID);
resp=new Uint8Array(fr.buffer||fr);
}catch{
try{
const fr2=await device.receiveFeatureReport(0);
resp=new Uint8Array(fr2.buffer||fr2);
}catch{}
}
if(!resp)throw new Error("no response");
return parseResponse(resp);
}
export async function readField(fieldId){
const pkt=makePacket(CMD_READ,fieldId,new Uint8Array(0));
return await sendPacketAndReadResponse(pkt);
}
export async function writeField(fieldId,payload){
const p=payload instanceof Uint8Array?payload:new Uint8Array(payload);
const pkt=makePacket(CMD_WRITE,fieldId,p);
return await sendPacketAndReadResponse(pkt);
}
export async function ping(){
const pkt=makePacket(CMD_PING,0,new Uint8Array([0xAA,0x55]));
return await sendPacketAndReadResponse(pkt);
}
export function u8(v){const b=new Uint8Array(1);b[0]=v&0xff;return b;}
export function u16le(v){const b=new Uint8Array(2);b[0]=v&0xff;b[1]=(v>>8)&0xff;return b;}
export function u32le(v){const b=new Uint8Array(4);b[0]=v&0xff;b[1]=(v>>8)&0xff;b[2]=(v>>16)&0xff;b[3]=(v>>24)&0xff;return b;}
export function colorToBytes(hex){
const h=String(hex).replace('#','').trim();
if(!/^[0-9a-fA-F]{6}$/.test(h))throw new Error("invalid color");
const r=parseInt(h.slice(0,2),16);const g=parseInt(h.slice(2,4),16);const b=parseInt(h.slice(4,6),16);
return new Uint8Array([r,g,b]);
}
export function bytesToColor(u8arr){
if(!u8arr||u8arr.length<3)return "#0a5bd7";
return "#"+[...u8arr.slice(0,3)].map(x=>x.toString(16).padStart(2,'0')).join('');
}
