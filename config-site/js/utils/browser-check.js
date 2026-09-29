export function checkBrowser(log){
  const ua = navigator.userAgent||'unknown';
  const hasHid = 'hid' in navigator;
  const isBad = !hasHid || /Firefox|Safari/i.test(ua) && !/Chrome|Edge|Chromium/i.test(ua) || /iPhone|iPad|iPod/i.test(ua);
  if(!isBad) return true;
  const overlay = document.getElementById('browserOverlay');
  if(overlay){
    const uaEl = overlay.querySelector('.overlay-ua');
    if(uaEl) uaEl.textContent = ua;
    overlay.classList.remove('hidden');
  }
  if(log) log('Browser not supported: '+ua);
  return false;
}
export function initBrowserOverlay(){
  const overlay = document.getElementById('browserOverlay');
  if(!overlay) return;
  overlay.querySelector('#closeOverlay')?.addEventListener('click',()=>{
    overlay.classList.add('hidden');
  });
}
