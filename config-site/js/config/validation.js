import {str2b,u16b} from './encoding.js';
import {hex2rgb} from '../utils/color.js';

export function validate(fid,rawStr,schema){
  const sch=schema[fid];
  if(!sch) return {ok:false,err:'Unknown field'};
  if(sch.readonly) return {ok:false,err:'Read only'};
  if(sch.type==='string'){
    if(rawStr.length>sch.max) return {ok:false,err:`Max ${sch.max} chars`};
    return {ok:true,data:str2b(rawStr)};
  }
  if(sch.type==='rgb'){
    if(!/^#[0-9a-fA-F]{6}$/.test(rawStr)) return {ok:false,err:'Use #rrggbb'};
    return {ok:true,data:new Uint8Array(hex2rgb(rawStr))};
  }
  if(sch.type==='u8' || sch.type==='bool'){
    const v=Number(rawStr);
    if(!Number.isInteger(v)) return {ok:false,err:'Integer needed'};
    if(sch.type==='bool' && (v===0||v===1)) return {ok:true,data:new Uint8Array([v])};
    if(v<sch.min||v>sch.max) return {ok:false,err:`${sch.min} to ${sch.max}`};
    return {ok:true,data:new Uint8Array([v])};
  }
  if(sch.type==='u16'){
    const v=Number(rawStr);
    if(!Number.isInteger(v)||v<sch.min||v>sch.max) return {ok:false,err:`${sch.min} to ${sch.max}`};
    return {ok:true,data:u16b(v)};
  }
  if(sch.type==='enum'){
    const v=Number(rawStr);
    if(!sch.values.includes(v)) return {ok:false,err:`Pick ${sch.values.join(', ')}`};
    if(fid===32||fid===33||fid===85) return {ok:true,data:u16b(v)};
    return {ok:true,data:new Uint8Array([v])};
  }
  if(sch.type==='combo'){
    if(!rawStr.trim()) return {ok:true,data:new Uint8Array([])};
    const parts=rawStr.split(',').map(s=>s.trim()).filter(Boolean);
    if(parts.length>8) return {ok:false,err:'Max 8 keys'};
    try{
      const bytes=parts.map(p=>{
        const n=Number(p);
        if(!Number.isInteger(n)||n<0||n>255) throw Error(p);
        return n;
      });
      return {ok:true,data:new Uint8Array(bytes)};
    }catch(e){return {ok:false,err:'Bad combo: '+e.message};}
  }
  return {ok:false,err:'Unknown type'};
}
