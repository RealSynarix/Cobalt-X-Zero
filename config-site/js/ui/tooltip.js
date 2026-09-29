import {$} from '../utils/helpers.js';
let tipEl=null;
let hoverTimer=null;
let currentTarget=null;

function ensureTip(){
  if(tipEl) return tipEl;
  tipEl=document.createElement('div');
  tipEl.className='tooltip';
  document.body.appendChild(tipEl);
  return tipEl;
}

function showTip(target, title, desc){
  const el=ensureTip();
  el.innerHTML=`<div class="tooltip-title">${title}</div><div>${desc}</div>`;
  const rect=target.getBoundingClientRect();
  el.classList.add('show');
  // position
  requestAnimationFrame(()=>{
    const tr=el.getBoundingClientRect();
    let top=rect.bottom+8;
    let left=rect.left;
    if(left+tr.width>window.innerWidth-12) left=window.innerWidth-tr.width-12;
    if(top+tr.height>window.innerHeight-12) top=rect.top-tr.height-8;
    if(top<8) top=8;
    el.style.top=top+window.scrollY+'px';
    el.style.left=left+window.scrollX+'px';
  });
}

function hideTip(){
  if(tipEl) tipEl.classList.remove('show');
  if(hoverTimer){clearTimeout(hoverTimer); hoverTimer=null;}
  currentTarget=null;
}

export function initTooltips(schema){
  document.addEventListener('mouseover',e=>{
    const label=e.target.closest('label');
    if(!label) return;
    const input=label.querySelector('input,select');
    if(!input) return;
    const fid=input.id?.replace('f','');
    if(!fid) return;
    const sch=schema[Number(fid)];
    if(!sch||!sch.desc) return;
    if(currentTarget===label) return;
    if(hoverTimer) clearTimeout(hoverTimer);
    currentTarget=label;
    hoverTimer=setTimeout(()=>{
      showTip(label, sch.label, sch.desc);
    },1000);
  });
  document.addEventListener('mouseout',e=>{
    const label=e.target.closest('label');
    if(!label) return;
    if(label===currentTarget){
      hideTip();
    }
  });
  document.addEventListener('mousemove',e=>{
    if(!currentTarget) return;
    // if moved far, reset timer? simple: if target changed, hide
    // if mouse moves but still over same label, keep timer unless already shown
    // if mouse moves quickly, hide and restart? we keep simple: on mousemove, if tooltip shown, follow? no, just keep.
    // If user moves mouse significantly within label, we reset to avoid flicker? ignore.
  });
  document.addEventListener('scroll',hideTip,true);
  document.addEventListener('keydown',hideTip);
}
