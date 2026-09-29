import {$ , $$} from '../utils/helpers.js';
export function initTabs(){
  const bar=$('#tabsBar');
  if(!bar) return;
  bar.addEventListener('click',e=>{
    const btn=e.target.closest('.tab-btn');
    if(!btn) return;
    const tab=btn.dataset.tab;
    if(!tab) return;
    setActiveTab(tab);
  });
  // default
  setActiveTab('profiles');
}
export function setActiveTab(name){
  $$('.tab-btn').forEach(b=>{b.classList.toggle('active', b.dataset.tab===name)});
  $$('.tab-panel').forEach(p=>{p.classList.toggle('active', p.id==='tab-'+name)});
  // save
  try{localStorage.setItem('cxz_tab',name)}catch{}
}
export function restoreTab(){
  try{
    const t=localStorage.getItem('cxz_tab');
    if(t) setActiveTab(t);
  }catch{}
}
