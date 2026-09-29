import {$ , $$} from '../utils/helpers.js';
export function initResponsive(){
  const modeEl=document.getElementById('f112');
  if(!modeEl) return;
  const update=()=>{
    const mode=Number(modeEl.value);
    $$('.pb7-conditional').forEach(el=>{
      const showFor=el.dataset.showFor;
      if(showFor===undefined) return;
      if(Number(showFor)===mode) el.classList.add('show');
      else el.classList.remove('show');
    });
  };
  modeEl.addEventListener('input',update);
  modeEl.addEventListener('change',update);
  update();
}
