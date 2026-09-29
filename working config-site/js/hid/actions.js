import {getDevice,setDevice,forgetAll,getFilters} from './connection.js';
import {sendWithRetry,CMD} from './protocol.js';
import {readAll,saveDirty,baseline,snapshotCurrent,checkDirty} from './transport.js';
import {setStatus,setStep} from '../ui/status.js';
import {setDirty} from '../ui/dirty-tracker.js';
import {SCHEMA} from '../config/schema.js';
import {validate} from '../config/validation.js';
import {ensureHardwareDefaultProfile,renderProfiles} from '../profiles/manager.js';
import {setActiveTab} from '../ui/tabs.js';
import {$} from '../utils/helpers.js';

export async function handleMainConnect(onLoad){
  const btn=$('#mainConnectBtn');
  const loader=$('#loader');
  const errEl=$('#connectError');
  if(errEl){errEl.classList.add('hidden'); errEl.textContent='';}
  btn?.classList.add('hidden');
  loader?.classList.remove('hidden');
  setStep('Checking WebHID support...');
  console.log('[Cobalt] Checking WebHID support...');
  if(!('hid' in navigator)){
    const msg='No WebHID - use Chrome or Edge on desktop';
    setStep(msg); console.log('[Cobalt] '+msg);
    if(errEl){errEl.textContent='Connection failed - '+msg; errEl.classList.remove('hidden');}
    loader?.classList.add('hidden'); btn?.classList.remove('hidden'); return;
  }
  try{
    setStep('Forgetting previous pairings...'); console.log('[Cobalt] Forgetting previous devices...');
    await forgetAll(); await new Promise(r=>setTimeout(r,200));
    setStep('Requesting device - pick Cobalt-X Zero in dialog...'); console.log('[Cobalt] requestDevice...');
    const picked=await navigator.hid.requestDevice({filters:getFilters()});
    if(!picked.length) throw Error('No device picked');
    const dev=picked[0]; setDevice(dev);
    console.log('[Cobalt] Picked '+(dev.productName||'Cobalt'));
    setStep(`Picked ${dev.productName||'Cobalt'} - opening...`);
    if(dev.opened) try{await dev.close()}catch{}
    await dev.open(); console.log('[Cobalt] Opened OK'); setStep('Opened - reading config from flash...');
    await new Promise(r=>setTimeout(r,250));
    await doReadAll();
    setStep('Creating snapshot...'); ensureHardwareDefaultProfile(baseline,SCHEMA);
    renderProfiles(onLoad,setStatus); setStep('Connected');
    await new Promise(r=>setTimeout(r,300));
    $('#connectPage')?.classList.add('hidden'); $('#configPage')?.classList.remove('hidden');
    setStatus('Connected - snapshot updated'); setActiveTab('profiles');
  }catch(e){
    console.error(e); console.log('[Cobalt] Connection FAIL '+e.name+' '+e.message);
    if(errEl){errEl.textContent=`Connection failed - ${e.message}`; errEl.classList.remove('hidden');}
    loader?.classList.add('hidden'); btn?.classList.remove('hidden'); setStep('Failed - check console');
  }
}

export async function handleDisconnect(){
  try{const d=getDevice(); if(d) await d.close()}catch{}
  setDevice(null);
  setStatus('Disconnected');
  $('#configPage')?.classList.add('hidden');
  $('#connectPage')?.classList.remove('hidden');
  $('#mainConnectBtn')?.classList.remove('hidden');
  $('#loader')?.classList.add('hidden');
  $('#connectError')?.classList.add('hidden');
}

export async function doReadAll(){
  const dev=getDevice(); if(!dev){setStatus('Connect mouse first'); return;}
  setStatus('Reading...'); await readAll(SCHEMA,setStep);
  snapshotCurrent(SCHEMA); checkDirty(SCHEMA,setDirty);
  setStatus('Read done');
}

export async function doSave(){
  const dev=getDevice(); if(!dev){setStatus('Connect mouse first'); return;}
  setStatus('Saving...'); await saveDirty(SCHEMA,validate,setDirty,setStatus);
  setStatus(`Saved. ${Object.keys(baseline).length} fields on device`);
}

export async function handleReset(onLoad){
  if(!confirm('Factory reset? This clears flash to defaults.')) return;
  const dev=getDevice(); if(!dev){setStatus('Connect mouse first'); return;}
  try{
    await sendWithRetry(CMD.RESET,0,new Uint8Array([]));
    console.log('[Cobalt] Reset OK'); setStatus('Reset OK - re-reading...');
    await doReadAll();
    ensureHardwareDefaultProfile(baseline,SCHEMA);
    renderProfiles(onLoad,setStatus);
  }catch(e){console.log('[Cobalt] Reset FAIL '+e.message); setStatus('Reset FAIL '+e.message);}
}

export async function handlePing(){
  const dev=getDevice(); if(!dev){setStatus('Connect mouse first'); return;}
  try{
    const r=await sendWithRetry(CMD.PING,0,new Uint8Array([]));
    setStatus('Ping ok - status '+r.status); console.log('[Cobalt] Ping OK');
  }catch(e){setStatus('Ping fail '+e.message); console.log('[Cobalt] Ping FAIL '+e.message);}
}
