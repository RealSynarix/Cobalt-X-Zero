import {loadProfiles, saveProfiles} from './storage.js';
import {$ , $$} from '../utils/helpers.js';
import {allFids} from '../config/fields.js';

function isHW(p){return p.id==='hardware_default'||p.isHardwareDefault}

export function ensureHardwareDefaultProfile(baseline, schema){
  const hwData={};
  for(const fid of allFids(schema)){
    if(baseline[fid]!==undefined) hwData[fid]=baseline[fid];
  }
  let profiles=loadProfiles();
  const hwProfile={
    id:'hardware_default',
    name:'Hardware Default',
    color:'#666666',
    data:hwData,
    created:new Date().toISOString(),
    isHardwareDefault:true
  };
  const idx=profiles.findIndex(p=>p.id==='hardware_default');
  if(idx>=0) profiles[idx]=hwProfile;
  else profiles.unshift(hwProfile);
  saveProfiles(profiles);
  console.log('[Cobalt] Hardware snapshot saved');
}

export function renderProfiles(onLoad, onStatus){
  const list=loadProfiles();
  const container=$('#profileList');
  if(!container) return;
  container.innerHTML='';
  if(list.length===0){
    container.innerHTML='<p class="muted">No profiles yet. Connect to create Hardware Default.</p>';
    return;
  }
  list.forEach(p=>{
    const hw=isHW(p);
    const div=document.createElement('div');
    div.className='profile'+(hw?' hardware':'');
    const letter=(p.name||'?')[0].toUpperCase();
    const hwProfile=list.find(x=>isHW(x));
    const count=hw ? 0 : Object.keys(p.data||{}).filter(fid=>!hwProfile || String(p.data[fid])!==String(hwProfile.data[fid])).length;
    const badge = hw ? '<span class="badge hw">Snapshot</span>' : `<span class="badge">${count} changed</span>`;
    const actions = hw ? '' : `<div class="profile-actions"><button data-act="load" data-id="${p.id}">Load</button><button data-act="delete" data-id="${p.id}" class="secondary">Delete</button></div>`;
    div.innerHTML=`<div class="icon" style="background:${p.color||'#333'}">${letter}</div><div class="profile-info"><div class="profile-name">${p.name} ${badge}</div><div class="profile-meta">${new Date(p.created).toLocaleString()}</div></div>${actions}`;
    container.appendChild(div);
  });
  container.querySelectorAll('button').forEach(b=>{
    b.onclick=()=>{
      const id=b.dataset.id; const act=b.dataset.act;
      const profiles=loadProfiles();
      const prof=profiles.find(x=>x.id===id);
      if(!prof) return;
      if(isHW(prof)) return;
      if(act==='load'){
        if(onLoad) onLoad(prof);
      }else if(act==='delete'){
        if(!confirm(`Delete ${prof.name}?`)) return;
        saveProfiles(profiles.filter(x=>x.id!==id));
        renderProfiles(onLoad, onStatus);
      }
    };
  });
}

export function createProfileFromCurrent(current, schema, validate, setStatus){
  const nameEl=$('#newProfileName');
  const colorEl=$('#newProfileColor');
  const name=nameEl?.value.trim();
  const color=colorEl?.value||'#0a5bd7';
  if(!name){setStatus&&setStatus('Profile name needed'); return false}
  if(name==='Hardware Default'){setStatus&&setStatus('Name reserved'); return false}
  const data={};
  for(const fid of Object.keys(schema).map(Number)){
    const raw=current[fid];
    if(raw===undefined) continue;
    const sch=schema[fid];
    if(sch.readonly) continue;
    const v=validate(fid,raw,schema);
    if(!v.ok){setStatus&&setStatus(`FID${fid} invalid: ${v.err}`); console.log(`[Cobalt] Profile save fail FID${fid} ${v.err}`); return false}
    data[fid]=raw;
  }
  const profiles=loadProfiles();
  const id=Date.now().toString(36)+Math.random().toString(36).slice(2,6);
  const source=document.getElementById('pioCode')?.value||'';
   profiles.push({id,name,color,data,source,created:new Date().toISOString()});
  saveProfiles(profiles);
  if(nameEl) nameEl.value='';
  console.log(`[Cobalt] Saved profile ${name}`);
  setStatus&&setStatus(`Saved ${name}`);
  return true;
}
