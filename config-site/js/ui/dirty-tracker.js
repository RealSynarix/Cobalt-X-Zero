import {dirty} from '../hid/transport.js';
import {$} from '../utils/helpers.js';

export function setDirty(fid,changed){
  const el=document.getElementById('f'+fid);
  if(changed){dirty.add(fid); el?.classList.add('dirty');}
  else {dirty.delete(fid); el?.classList.remove('dirty');}
  updateSaveBtn();
}
export function updateSaveBtn(){
  const b=$('#saveBtn');
  if(!b) return;
  b.textContent=`Save changed (${dirty.size})`;
  b.disabled=dirty.size===0;
  b.style.opacity = dirty.size===0 ? '.35' : '1';
  if(dirty.size>0) b.classList.add('has-dirty');
  else b.classList.remove('has-dirty');
}
export function clearDirtyUI(){
  document.querySelectorAll('.dirty').forEach(el=>el.classList.remove('dirty'));
  dirty.clear();
  updateSaveBtn();
}
