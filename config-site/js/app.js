import {SCHEMA} from './config/schema.js';
import {validate} from './config/validation.js';
import {setStatus} from './ui/status.js';
import {initTabs} from './ui/tabs.js';
import {initResponsive} from './ui/responsive.js';
import {setDirty,updateSaveBtn} from './ui/dirty-tracker.js';
import {snapshotCurrent,checkDirty,current} from './hid/transport.js';
import {initImportExport} from './profiles/import-export.js';
import {renderProfiles,createProfileFromCurrent} from './profiles/manager.js';
import {checkBrowser,initBrowserOverlay} from './utils/browser-check.js';
import {handleMainConnect,handleDisconnect,doReadAll,doSave,handleReset,handlePing} from './hid/actions.js';
import {initTooltips} from './ui/tooltip.js';
import {initModals,openModal,closeModal} from './ui/modal.js';
import {$ , $$} from './utils/helpers.js';
import {compilePIO,exampleSource,hexChunks} from './config/pio-compiler.js';

export function initApp(){
  initBrowserOverlay();
  initTabs();
  initResponsive();
  initImportExport();
  initModals();
  initTooltips(SCHEMA);
  checkBrowser();
  bindFields();
  renderProfiles(handleLoad,setStatus);
  updateSaveBtn();
  bindButtons();
  console.log('[Cobalt] Ready - connect to load config');
}

function bindFields(){
  for(const fid of Object.keys(SCHEMA).map(Number)){
    const el=document.getElementById('f'+fid);
    if(!el) continue;
    const onChange=()=>{
      snapshotCurrent(SCHEMA);
      checkDirty(SCHEMA,setDirty);
      if(fid===112){
        const mode=Number(el.value);
        $('#pioCodePane')?.classList.toggle('hidden',mode!==5);
      }
      const sw=document.querySelector(`[data-swatch-for='f${fid}']`);
      if(sw) sw.style.background=el.value;
    };
    el.addEventListener('input',onChange);
    el.addEventListener('change',onChange);
  }
  const pioMode=$('#f112');
  if(pioMode) $('#pioCodePane')?.classList.toggle('hidden',Number(pioMode.value)!==5);
}

function handleLoad(prof){
  for(const [fid,val] of Object.entries(prof.data)){
    const el=document.getElementById('f'+fid);
    if(!el) continue;
    if(SCHEMA[fid]?.readonly) continue;
    el.value=val;
    const sw=document.querySelector(`[data-swatch-for='f${fid}']`);
    if(sw) sw.style.background=val;
  }
  const code=document.getElementById('pioCode');
  if(code&&prof.source) code.value=prof.source;
  snapshotCurrent(SCHEMA);
  checkDirty(SCHEMA,setDirty);
  console.log(`[Cobalt] Loaded ${prof.name}`);
  setStatus(`Loaded ${prof.name} - save to write to mouse`);
}

function bindButtons(){
  $('#mainConnectBtn')?.addEventListener('click',()=>handleMainConnect(handleLoad));
  $('#disconnectBtn')?.addEventListener('click',handleDisconnect);
  $('#readBtn')?.addEventListener('click',doReadAll);
  $('#saveBtn')?.addEventListener('click',doSave);
  $('#resetBtn')?.addEventListener('click',()=>handleReset(handleLoad));
  $('#pingBtn')?.addEventListener('click',handlePing);
  $('#addProfileBtn')?.addEventListener('click',()=>openModal('profileModal'));
  $('#createProfileBtn')?.addEventListener('click',()=>{
    const ok=createProfileFromCurrent(current,SCHEMA,validate,setStatus);
    if(ok){
      closeModal('profileModal');
      renderProfiles(handleLoad,setStatus);
    }
  });
  $('#loadPioExample')?.addEventListener('click',()=>{
    const n=$('#pioExample')?.value||'custom';
    const mode=$('#f112');
    if(mode){mode.value='5';mode.dispatchEvent(new Event('change'));}
    const code=$('#pioCode');
    if(code) code.value=exampleSource(n);
    setStatus(`Loaded PIO example: ${n}`);
  });
  $('#compilePio')?.addEventListener('click',()=>{
    try{
      const mode=$('#f112');
      if(mode){mode.value='5';mode.dispatchEvent(new Event('change'));}
      const bytes=compilePIO($('#pioCode')?.value||'');
      const chunks=hexChunks(bytes);
      chunks.forEach((hex,i)=>{
        const el=$('#f'+(119+i));
        if(el) el.value=hex;
      });
      snapshotCurrent(SCHEMA);
      for(let i=0;i<8;i++) setDirty(119+i,true);
      const st=$('#pioCompileStatus');
      if(st) st.textContent=`Compiled ${bytes[2]|(bytes[3]<<8)} bytes of PIO bytecode. Stage is ready; press Save changed to write it to the mouse.`;
      setStatus('PIO program compiled and staged');
    }catch(e){
      const st=$('#pioCompileStatus');
      if(st) st.textContent=`Compile error: ${e.message}`;
      setStatus(`PIO compile failed: ${e.message}`);
    }
  });
}
