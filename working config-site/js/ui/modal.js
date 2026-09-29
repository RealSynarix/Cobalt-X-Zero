import {$} from '../utils/helpers.js';
export function openModal(id){
  const el=document.getElementById(id);
  if(el) el.classList.remove('hidden');
}
export function closeModal(id){
  const el=document.getElementById(id);
  if(el) el.classList.add('hidden');
}
export function initModals(){
  document.querySelectorAll('[data-close-modal]').forEach(btn=>{
    btn.addEventListener('click',()=>{
      const id=btn.dataset.closeModal;
      closeModal(id);
    });
  });
  document.querySelectorAll('.modal-backdrop').forEach(bd=>{
    bd.addEventListener('click',e=>{
      if(e.target===bd) bd.classList.add('hidden');
    });
  });
}
