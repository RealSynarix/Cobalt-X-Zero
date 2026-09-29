import {$} from '../utils/helpers.js';
export function setStatus(m){
  const el=$('#status');
  if(el) el.textContent=m;
}
export function setStep(t){
  const el=$('#stepText');
  if(el) el.textContent=t;
}
