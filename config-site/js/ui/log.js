import {$} from '../utils/helpers.js';
let logEl=null;
export function initLog(){
  logEl=$('#log');
  const head=$('#logHead');
  const wrap=$('#logWrap');
  head?.addEventListener('click',()=>{wrap?.classList.toggle('collapsed')});
  $('#clearLog')?.addEventListener('click',(e)=>{e.stopPropagation(); if(logEl) logEl.textContent='';});
  $('#toggleLog')?.addEventListener('click',(e)=>{e.stopPropagation(); wrap?.classList.toggle('collapsed')});
}
export function log(m){
  console.log('[Cobalt] '+m);
  if(!logEl) logEl=document.getElementById('log');
  if(!logEl) return;
  const time=new Date().toLocaleTimeString();
  logEl.textContent += `${time}  ${m}\n`;
  logEl.scrollTop=logEl.scrollHeight;
}
