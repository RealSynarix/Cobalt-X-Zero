export let device=null;export let connected=false;
const REPORT_ID=2;
export function makePacket(cmd,fid,payload){
const buf=new Uint8Array(64);
const p=payload instanceof Uint8Array?payload:new Uint8Array(payload||[]);
let len=p.length;if(len>60)len=60;
buf[0]=cmd&0xff;buf[1]=fid&0xff;buf[2]=len&0xff;buf[3]=(len>>8)&0xff;
buf.set(p.subarray(0,len),4);
return buf;
}
export function encodeString(s,maxLen){const enc=new TextEncoder();const b=enc.encode(s);if(maxLen&&b.length>maxLen)return b.slice(0,maxLen);return b;}
export function decodeString(u8){try{const dec=new TextDecoder();let end=u8.length;for(let i=0;i<u8.length;i++){if(u8[i]===0){end=i;break;}}return dec.decode(u8.slice(0,end));}catch{return "";}}
export async function openDevice(d){
if(!d)throw new Error("no device");
if(!d.opened)await d.open();
device=d;connected=true;
return true;
}
export function parseResponse(raw){
const data=raw instanceof Uint8Array?raw:new Uint8Array(raw);
let off=0;if(data[0]===REPORT_ID)off=1;
if(data.length<5+off)return {cmd:0,fieldId:0,len:0,status:255,payload:new Uint8Array(0),raw:data};
return {cmd:data[off],fieldId:data[off+1],len:data[off+2]|(data[off+3]<<8),status:data[off+4],payload:data.slice(off+5,off+5+(data[off+2]|(data[off+3]<<8))),raw:data};
}
export async function sendPacketAndReadResponse(packet){
if(!device||!device.opened)throw new Error("No device opened");
const toSend=packet;
let sent=false;
try{await device.sendReport(REPORT_ID,toSend);sent=true;}catch(e){console.log("sendReport2 fail",e);}
if(!sent){try{await device.sendReport(0,toSend);sent=true;}catch(e){console.log("sendReport0 fail",e);}}
if(!sent){try{await device.sendFeatureReport(REPORT_ID,toSend);sent=true;}catch(e){console.log("sendFeatureReport2 fail",e);}}
if(!sent)throw new Error("All send methods failed - device may be protected (standard mouse blocked by Chrome)");
await new Promise(r=>setTimeout(r,90));
let resp=null;
try{const fr=await device.receiveFeatureReport(REPORT_ID);resp=new Uint8Array(fr.buffer||fr);}catch(e){console.log("receiveFeatureReport2 fail",e);}
if(!resp){try{const fr2=await device.receiveFeatureReport(0);resp=new Uint8Array(fr2.buffer||fr2);}catch(e){console.log("receiveFeatureReport0 fail",e);}}
if(!resp)throw new Error("No response - check firmware has vendor 0xFF00 page, not just boot mouse");
return parseResponse(resp);
}
export async function readField(fid){return await sendPacketAndReadResponse(makePacket(1,fid,new Uint8Array(0)));}
export async function writeField(fid,p){return await sendPacketAndReadResponse(makePacket(2,fid,p instanceof Uint8Array?p:new Uint8Array(p)));}
export async function ping(){return await sendPacketAndReadResponse(makePacket(5,0,new Uint8Array([170,85])));}
export function u8(v){return new Uint8Array([v&0xff]);}
export function u16le(v){return new Uint8Array([v&0xff,(v>>8)&0xff]);}
export function colorToBytes(hex){const h=String(hex).replace('#','').trim();if(!/^[0-9a-fA-F]{6}$/.test(h))throw new Error("invalid color");return new Uint8Array([parseInt(h.slice(0,2),16),parseInt(h.slice(2,4),16),parseInt(h.slice(4,6),16)]);}
export function bytesToColor(a){if(!a||a.length<3)return "#0a5bd7";return "#"+[...a.slice(0,3)].map(x=>x.toString(16).padStart(2,'0')).join('');}
