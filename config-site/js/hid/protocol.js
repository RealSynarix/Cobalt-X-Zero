import {enc,dec} from '../config/encoding.js';
import {getDevice} from './connection.js';
export const CMD={READ:1,WRITE:2,RESET:4,PING:5};
export const STATUS={OK:0,BUSY:3};
export const RID=2;

export async function sendWithRetry(cmd,fid,data,log,retries=3){
  const dev=getDevice();
  if(!dev) throw Error('No device');
  for(let i=0;i<=retries;i++){
    try{
      const out=enc(cmd,fid,data);
      await dev.sendFeatureReport(RID,out);
      await new Promise(r=>setTimeout(r,70));
      const raw=await dev.receiveFeatureReport(RID);
      const rr=dec(raw);
      if(rr.rawLen!==undefined && rr.rawLen<5) throw Error('short reply '+rr.rawLen);
      if(rr.status===3){
        log&&log(`FID${fid} busy retry ${i+1}`);
        await new Promise(r=>setTimeout(r,110));
        continue;
      }
      if(rr.status!==0) throw Error('dev status '+rr.status);
      if(rr.cmd!==cmd||rr.fid!==fid) log&&log(`WARN cmd/fid mismatch got ${rr.cmd}/${rr.fid} exp ${cmd}/${fid}`);
      return rr;
    }catch(e){
      log&&log(`send ${cmd}/${fid} attempt ${i+1} FAIL ${e.message}`);
      if(i===retries) throw e;
      await new Promise(r=>setTimeout(r,160));
      try{const d=getDevice(); if(d&&!d.opened) await d.open()}catch{}
    }
  }
}
