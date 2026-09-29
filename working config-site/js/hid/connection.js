export const VID=0x1209, PID=0xC0BA, RID=2;
export let device=null;
export function setDevice(d){device=d}
export function getDevice(){return device}
export async function forgetAll(){
  try{
    if(!('hid' in navigator)) return;
    const ds=await navigator.hid.getDevices();
    for(const d of ds){
      if(d.vendorId===VID && d.productId===PID){
        try{await d.forget(); console.log('[Cobalt] Forgot '+(d.productName||'device'))}catch(e){console.log('[Cobalt] Forget fail '+e.message)}
      }
    }
  }catch(e){console.log('[Cobalt] forgetAll '+e.message)}
}
export function getFilters(){
  return [{vendorId:VID,productId:PID,usagePage:0xFF00},{vendorId:VID,productId:PID}];
}
