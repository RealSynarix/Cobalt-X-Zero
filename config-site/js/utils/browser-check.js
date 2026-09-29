export function checkBrowser(){
  const ua = navigator.userAgent||'unknown';
  const hasHid = 'hid' in navigator;
  const isBad = !hasHid || /Firefox|Safari/i.test(ua) && !/Chrome|Edge|Chromium/i.test(ua) || /iPhone|iPad|iPod/i.test(ua);
  if(!isBad) return true;
  const overlay = document.getElementById('browserOverlay');
  if(overlay){
    const uaEl = document.getElementById('overlayUa');
    if(uaEl) uaEl.textContent = ua;
    overlay.classList.remove('hidden');
  }
  console.log('[Cobalt] Browser not supported: '+ua);
  return false;
}
export function initBrowserOverlay(){
  const overlay = document.getElementById('browserOverlay');
  if(!overlay) return;
  document.getElementById('closeOverlay')?.addEventListener('click',()=>{
    overlay.classList.add('hidden');
  });
}
