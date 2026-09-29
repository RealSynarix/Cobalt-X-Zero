import {getDevice,setDevice,forgetAll,getFilters} from './connection.js';
import {sendWithRetry,CMD} from './protocol.js';
import {readAll,saveDirty,baseline,current,snapshotCurrent,checkDirty} from './transport.js';
import {setStatus,setStep} from '../ui/status.js';
import {log} from '../ui/log.js';
import {setDirty} from '../ui/dirty-tracker.js';
import {SCHEMA} from '../config/schema.js';
import {validate} from '../config/validation.js';
import {ensureHardwareDefaultProfile,renderProfiles} from '../profiles/manager.js';
import {setActiveTab} from '../ui/tabs.js';
import {$} from '../utils/helpers.js';

let demoMode=false;
let demoData={};

function buildDemo(){
  const d={};
  d[1]='Cobalt-X Zero'; d[2]='Synarix'; d[3]='CXZ-000001'; d[4]='1.0.0';
  d[16]='1'; d[17]='1'; d[18]='1'; d[19]='1'; d[20]='1'; d[21]='1';
  d[32]='800'; d[33]='1000'; d[34]='2'; d[35]='80'; d[36]='0'; d[37]='0'; d[38]='0';
  d[48]='4'; d[49]='4'; d[50]='4'; d[51]='4'; d[52]='8'; d[53]='0';
  d[64]='1'; d[65]='0'; d[66]='1';
  d[80]='1'; d[81]='1'; d[82]='0'; d[83]='4'; d[84]='1'; d[85]='40';
  d[96]='#0A5BD7'; d[97]='#FF3333'; d[98]='128'; d[99]='1'; d[100]='100';
  d[112]='0'; d[113]='0'; d[114]='0'; d[115]='0'; d[116]=''; d[117]='0';
  return d;
}
export function isDemo(){return demoMode}

export async function handleMainConnect(onLoad){
  const btn=$('#mainConnectBtn');
  const loader=$('#loader');
  const errEl=$('#connectError');
  if(errEl){errEl.classList.add('hidden'); errEl.textContent='';}
  btn?.classList.add('hidden');
  $('#openDemoBtn')?.classList.add('hidden');
  loader?.classList.remove('hidden');
  setStep('Checking WebHID support...'); log('Checking WebHID support...');
  if(!('hid' in navigator)){
    const msg='No WebHID - use Chrome or Edge on desktop';
    setStep(msg); log(msg);
    if(errEl){errEl.textContent='Connection failed - '+msg; errEl.classList.remove('hidden');}
    loader?.classList.add('hidden'); btn?.classList.remove('hidden');
    $('#openDemoBtn')?.classList.remove('hidden'); return;
  }
  try{
    setStep('Forgetting previous pairings...'); log('Forgetting previous devices...');
    await forgetAll(log); await new Promise(r=>setTimeout(r,200));
    setStep('Requesting device - pick Cobalt-X Zero in dialog...'); log('requestDevice...');
    const picked=await navigator.hid.requestDevice({filters:getFilters()});
    if(!picked.length) throw Error('No device picked');
    const dev=picked[0]; setDevice(dev);
    log('Picked '+(dev.productName||'Cobalt')); setStep(`Picked ${dev.productName||'Cobalt'} - opening...`);
    if(dev.opened) try{await dev.close()}catch{}
    await dev.open(); log('Opened OK'); setStep('Opened - reading config from flash...');
    await new Promise(r=>setTimeout(r,250));
    await doReadAll();
    setStep('Creating snapshot...'); ensureHardwareDefaultProfile(baseline,log,SCHEMA);
    renderProfiles(onLoad,setStatus); setStep('Connected');
    await new Promise(r=>setTimeout(r,300));
    $('#connectPage')?.classList.add('hidden'); $('#configPage')?.classList.remove('hidden');
    setStatus('Connected - snapshot updated'); setActiveTab('profiles');
  }catch(e){
    console.error(e); log('Connection FAIL '+e.name+' '+e.message);
    if(errEl){errEl.textContent=`Connection failed - ${e.message}`; errEl.classList.remove('hidden');}
    loader?.classList.add('hidden'); btn?.classList.remove('hidden');
    $('#openDemoBtn')?.classList.remove('hidden'); setStep('Failed - check console');
  }
}

export function openDemo(onLoad){
  demoMode=true; demoData=buildDemo();
  for(const fid of Object.keys(demoData)){
    const el=document.getElementById('f'+fid);
    if(el) el.value=demoData[fid];
    baseline[fid]=demoData[fid];
    const sw=document.querySelector(`[data-swatch-for='f${fid}']`);
    if(sw) sw.style.background=demoData[fid];
  }
  snapshotCurrent(SCHEMA); checkDirty(SCHEMA,setDirty);
  renderProfiles(onLoad,setStatus);
  $('#connectPage')?.classList.add('hidden'); $('#configPage')?.classList.remove('hidden');
  setStatus('Demo session. Same UI as live mouse, nothing written to hardware.');
  setActiveTab('device'); log('Demo session started');
}

export async function handleDisconnect(){
  try{const d=getDevice(); if(d) await d.close()}catch{}
  setDevice(null); demoMode=false;
  setStatus('Disconnected');
  $('#configPage')?.classList.add('hidden');
  $('#connectPage')?.classList.remove('hidden');
  $('#mainConnectBtn')?.classList.remove('hidden');
  $('#openDemoBtn')?.classList.remove('hidden');
  $('#loader')?.classList.add('hidden');
  $('#connectError')?.classList.add('hidden');
}

export async function doReadAll(){
  if(demoMode){
    for(const fid of Object.keys(demoData)){
      const el=document.getElementById('f'+fid);
      if(el) el.value=demoData[fid];
      baseline[fid]=demoData[fid];
    }
    snapshotCurrent(SCHEMA); checkDirty(SCHEMA,setDirty);
    setStatus('Demo re-read done'); log('Demo re-read'); return;
  }
  const dev=getDevice(); if(!dev){setStatus('Connect mouse first'); return;}
  setStatus('Reading...'); await readAll(SCHEMA,setStep,log);
  snapshotCurrent(SCHEMA); checkDirty(SCHEMA,setDirty);
  setStatus('Read done');
}

export async function doSave(){
  if(demoMode){
    for(const fid of Object.keys(SCHEMA).map(Number)){
      const el=document.getElementById('f'+fid); if(!el) continue;
      demoData[fid]=el.value; baseline[fid]=el.value;
    }
    snapshotCurrent(SCHEMA); checkDirty(SCHEMA,setDirty);
    setStatus('Demo saved locally'); log('Demo saved'); return;
  }
  const dev=getDevice(); if(!dev){setStatus('Connect mouse first'); return;}
  setStatus('Saving...'); await saveDirty(SCHEMA,validate,setDirty,log,setStatus);
  setStatus(`Saved. ${Object.keys(baseline).length} fields on device`);
}

export async function handleReset(onLoad){
  if(!confirm('Factory reset? This clears flash to defaults.')) return;
  if(demoMode){
    demoData=buildDemo();
    for(const fid of Object.keys(demoData)){
      const el=document.getElementById('f'+fid); if(el) el.value=demoData[fid]; baseline[fid]=demoData[fid];
    }
    snapshotCurrent(SCHEMA); checkDirty(SCHEMA,setDirty);
    setStatus('Demo reset to defaults'); return;
  }
  const dev=getDevice(); if(!dev){setStatus('Connect mouse first'); return;}
  try{
    await sendWithRetry(CMD.RESET,0,new Uint8Array([]),log);
    log('Reset OK'); setStatus('Reset OK - re-reading...');
    await doReadAll();
    ensureHardwareDefaultProfile(baseline,log,SCHEMA);
    renderProfiles(onLoad,setStatus);
  }catch(e){log('Reset FAIL '+e.message); setStatus('Reset FAIL '+e.message);}
}

export async function handlePing(){
  if(demoMode){setStatus('Ping ok - demo'); log('Ping demo ok'); return;}
  const dev=getDevice(); if(!dev){setStatus('Connect mouse first'); return;}
  try{
    const r=await sendWithRetry(CMD.PING,0,new Uint8Array([]),log);
    setStatus('Ping ok - status '+r.status); log('Ping OK');
  }catch(e){setStatus('Ping fail '+e.message); log('Ping FAIL '+e.message);}
}
