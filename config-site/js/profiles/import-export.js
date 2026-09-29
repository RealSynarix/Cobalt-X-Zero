import {loadProfiles, saveProfiles} from './storage.js';
import {renderProfiles} from './manager.js';
import {$} from '../utils/helpers.js';

export function initImportExport(){
  $('#exportProfiles')?.addEventListener('click',()=>{
    const data=JSON.stringify(loadProfiles(),null,2);
    const blob=new Blob([data],{type:'application/json'});
    const url=URL.createObjectURL(blob);
    const a=document.createElement('a'); a.href=url; a.download='cobalt_profiles.json'; a.click();
    setTimeout(()=>URL.revokeObjectURL(url),1000);
  });
  $('#importProfiles')?.addEventListener('click',()=>$('#importFile')?.click());
  $('#importFile')?.addEventListener('change',async e=>{
    const f=e.target.files[0]; if(!f) return;
    try{
      const txt=await f.text(); const arr=JSON.parse(txt);
      if(!Array.isArray(arr)) throw Error('Not array');
      let existing=loadProfiles();
      const hw=existing.find(p=>p.id==='hardware_default');
      const filtered=arr.filter(p=>p.id!=='hardware_default');
      if(hw) filtered.unshift(hw);
      saveProfiles(filtered);
      renderProfiles();
      console.log('[Cobalt] Imported '+arr.length+' profiles');
    }catch(err){console.log('[Cobalt] Import fail '+err.message)}
    e.target.value='';
  });
}
