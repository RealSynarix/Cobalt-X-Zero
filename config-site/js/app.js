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
        $$('.pb7-conditional').forEach(c=>{
          const sf=c.dataset.showFor;
          if(sf!==undefined) c.classList.toggle('show', Number(sf)===mode);
        });
      }
      const sw=document.querySelector(`[data-swatch-for='f${fid}']`);
      if(sw) sw.style.background=el.value;
    };
    el.addEventListener('input',onChange);
    el.addEventListener('change',onChange);
  }
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
}
