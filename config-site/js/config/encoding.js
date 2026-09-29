import {rgb2hex} from '../utils/color.js';
export function enc(cmd,fid,data){
  const b=new Uint8Array(64);
  b[0]=cmd; b[1]=fid; b[2]=data.length&255; b[3]=data.length>>8;
  b.set(data,4);
  return b;
}
export function dec(raw){
  let d;
  if(raw instanceof DataView) d=new Uint8Array(raw.buffer,raw.byteOffset,raw.byteLength);
  else if(raw instanceof Uint8Array) d=raw;
  else d=new Uint8Array(raw);
  if(d.length>=6 && d[0]===2 && [1,2,4,5].includes(d[1])) d=d.slice(1);
  if(d.length<5) return {cmd:0,fid:0,len:0,status:99,payload:new Uint8Array([]),rawLen:d.length};
  const len=d[2]|(d[3]<<8);
  return {cmd:d[0],fid:d[1],len,status:d[4],payload:d.slice(5,5+len)};
}
export function str2b(s){return new TextEncoder().encode(s)}
export function b2str(b){try{return new TextDecoder().decode(b).replace(/\0+$/,'')}catch{return''}}
export function u16b(v){return new Uint8Array([v&255,v>>8])}
export function b2u16(b){return b[0]|(b[1]<<8)}
export function payloadToVal(fid,payload){
  if([96,97].includes(fid)){
    if(payload.length>=3) return rgb2hex(payload[0],payload[1],payload[2]);
    return '#000000';
  }
  if([32,33,85].includes(fid)){
    if(payload.length>=2) return (payload[0]|(payload[1]<<8)).toString();
    return payload[0]?.toString()||'0';
  }
  if([1,2,3,4].includes(fid)) return b2str(payload);
  if([116].includes(fid)){
    if(payload.length===0) return '';
    return Array.from(payload).join(',');
  }
  if(payload.length>=1) return payload[0].toString();
  return '';
}
