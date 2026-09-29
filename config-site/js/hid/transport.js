import {CMD} from './protocol.js';
import {sendWithRetry} from './protocol.js';
import {payloadToVal} from '../config/encoding.js';
import {allFids, fieldEl} from '../config/fields.js';

export let baseline={}, current={}, dirty=new Set();

export function snapshotCurrent(schema){
  current={};
  for(const fid of allFids(schema)){
    const el=fieldEl(fid); if(!el) continue;
    current[fid]=el.value;
  }
}
export function checkDirty(schema, setDirty){
  for(const fid of allFids(schema)){
    if(baseline[fid]===undefined) continue;
    if(current[fid]!==baseline[fid]) setDirty(fid,true);
    else setDirty(fid,false);
  }
}

export async function readAll(schema, setStep, log){
  baseline={}; current={}; dirty.clear();
  document.querySelectorAll('.dirty').forEach(el=>el.classList.remove('dirty'));
  const ids=allFids(schema);
  for(const fid of ids){
    try{
      const r=await sendWithRetry(CMD.READ,fid,new Uint8Array([]),log);
      const val=payloadToVal(fid,r.payload);
      const el=fieldEl(fid);
      if(el) el.value=val;
      baseline[fid]=val;
      log&&log(`Read FID${fid} OK = ${val}`);
      setStep&&setStep(`Reading ${fid} - ${schema[fid]?.label||''} = ${val}`);
    }catch(e){log&&log(`Read FID${fid} FAIL ${e.message}`)}
    await new Promise(r=>setTimeout(r,35));
  }
  snapshotCurrent(schema);
}

export async function saveDirty(schema, validate, setDirtyFn, log, setStatus){
  snapshotCurrent(schema);
  if(dirty.size===0){log&&log('Nothing changed'); return}
  log&&log(`Saving ${dirty.size} changed: ${Array.from(dirty).join(',')}`);
  for(const fid of Array.from(dirty)){
    const el=fieldEl(fid);
    const raw=el?.value??'';
    const v=validate(fid,raw,schema);
    if(!v.ok){log&&log(`Validate FID${fid} FAIL ${v.err}`); setStatus&&setStatus(`FID${fid} invalid: ${v.err}`); continue;}
    try{
      await sendWithRetry(CMD.WRITE,fid,v.data,log);
      log&&log(`Saved FID${fid} OK`);
      baseline[fid]=current[fid];
      setDirtyFn(fid,false);
    }catch(e){log&&log(`Save FID${fid} FAIL ${e.message}`)}
    await new Promise(r=>setTimeout(r,50));
  }
}
