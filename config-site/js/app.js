import { device, openDevice, readField, writeField, ping } from "./hid.js";
import { FIELD_META, applyFieldToUI, gatherAllFields, loadConfigToUI } from "./config.js";
import { saveProfile, loadProfile, deleteProfile, refreshProfileUI } from "./profiles.js";
const VID=0x1209;const PID=0xC0BA;
let isConnecting=false;let connectedDevice=null;
const $=s=>document.querySelector(s);
function setStatus(m){$("#connect-status").textContent=m||"";console.log("[status]",m);}
function setState(m){$("#device-state").textContent=m;}
async function listPaired(){
try{
const devs=await navigator.hid.getDevices();
const el=$("#paired-list");
if(!devs.length){el.textContent="No paired devices";return;}
el.textContent=devs.map(d=>`${d.productName||'unknown'} ${d.vendorId.toString(16)}:${d.productId.toString(16)}${d.opened?' opened':''}`).join(' | ');
if(devs.some(d=>d.vendorId===0x4F3)){$("#linux-help").style.display="block";}
}catch(e){$("#paired-list").textContent=e.message;}
}
async function forgetAll(){
try{
const devs=await navigator.hid.getDevices();
for(const d of devs){try{await d.forget();}catch{}}
await listPaired();
setStatus("Forgot all paired. Now click Connect again and select ONLY Cobalt-X Zero");
}catch(e){setStatus(e.message);}
}
async function tryExisting(){
try{
const devs=await navigator.hid.getDevices();
const found=devs.find(d=>d.vendorId===VID&&d.productId===PID);
if(!found){setStatus("No paired Cobalt found. Need to request.");return null;}
setStatus(`Found paired ${found.productName} trying open...`);
try{
if(!found.opened)await found.open();
await openDevice(found);
connectedDevice=found;
setStatus(`Opened paired ${found.productName}`);
return found;
}catch(e){
console.error(e);
setStatus(`Paired open failed: ${e.name} ${e.message}\nIf SecurityError: Chrome blocks standard mice. Firmware must expose vendor 0xFF00 page (not just boot mouse).\nIf NotAllowedError/Access denied on Linux: missing udev rules. See below.\nELAN device is touchpad accidentally paired - use Forget All.`);
$("#linux-help").style.display="block";
return null;
}
}catch(e){setStatus(`getDevices error: ${e.message}`);return null;}
}
async function requestAndOpen(){
const filters=[{vendorId:VID,productId:PID}];
let devs;
try{
setStatus("Opening chooser...");
devs=await navigator.hid.requestDevice({filters});
}catch(e){
console.error(e);
if(e.name==="NotAllowedError"){setStatus("Chooser dismissed or blocked. Click Connect again.\nIf you selected mouse and still dismissed: Chrome may have blocked it as protected mouse (needs vendor page). Check firmware.");}
else{setStatus(`requestDevice failed: ${e.name} ${e.message}`);}
return null;
}
if(!devs||!devs.length){setStatus("No device selected. Click Connect again.");return null;}
const d=devs.find(x=>x.vendorId===VID&&x.productId===PID)||devs[0];
setStatus(`Selected ${d.productName||'device'} ${d.vendorId.toString(16)}:${d.productId.toString(16)} trying open...`);
try{
if(!d.opened)await d.open();
await openDevice(d);
connectedDevice=d;
setStatus(`Opened ${d.productName||'device'}`);
return d;
}catch(e){
console.error(e);
let extra="";
if(e.name==="SecurityError"){extra="\nSecurityError: Chrome blocks standard mice/keyboards from WebHID. Your firmware enumerates as boot mouse only. Must include vendor 0xFF00 interface with feature reports. Flash fixed firmware.";}
if(e.message&&e.message.toLowerCase().includes("access denied")){extra+="\nAccess denied on Linux: add udev rule /etc/udev/rules.d/99-cobalt.rules:\nSUBSYSTEM==\"hidraw\", ATTRS{idVendor}==\"1209\", ATTRS{idProduct}==\"c0ba\", MODE=\"0666\"\nThen sudo udevadm control --reload-rules && sudo udevadm trigger and replug.";$("#linux-help").style.display="block";}
setStatus(`Open failed: ${e.name} ${e.message}${extra}`);
return null;
}
}
async function onConnected(){
setStatus("Connected, ping...");
$("#connect-page").classList.add("hidden");
$("#app-page").classList.remove("hidden");
setState(`Connected ${VID.toString(16)}:${PID.toString(16)}`);
try{const p=await ping();setStatus(`Ping OK status=${p.status}`);}catch(e){setStatus(`Ping failed: ${e.message}. Will try read anyway.`);}
await readAll();
refreshProfileUI();
}
async function readAll(){
const ids=Object.keys(FIELD_META).map(n=>parseInt(n,10));
const txt=$("#apply-text");
for(let i=0;i<ids.length;i++){
const fid=ids[i];
try{const res=await readField(fid);if(res&&res.status===0&&res.payload)applyFieldToUI(fid,res.payload);}catch(e){console.log("read fail",fid,e);}
if(txt)txt.textContent=`Reading ${Math.round(((i+1)/ids.length)*100)}%`;
await new Promise(r=>setTimeout(r,15));
}
if(txt)txt.textContent="Ready";
$("#f-vid").value="0x"+VID.toString(16).toUpperCase();
$("#f-pid").value="0x"+PID.toString(16).toUpperCase();
}
async function applyAll(){
const btn=$("#apply-all");const prog=document.querySelector(".progress-fill");const txt=$("#apply-text");
let fields;
try{fields=gatherAllFields();}catch(e){alert(e.message);return;}
if(!fields.length)return;
btn.disabled=true;
let ok=0,fail=0;
for(let i=0;i<fields.length;i++){
const {fieldId,payload}=fields[i];
try{const res=await writeField(fieldId,payload);if(res.status===0)ok++;else fail++;}catch{fail++;}
const pct=Math.round(((i+1)/fields.length)*100);
if(prog)prog.style.width=pct+"%";
if(txt)txt.textContent=`${pct}% ok=${ok} fail=${fail}`;
await new Promise(r=>setTimeout(r,50));
}
btn.disabled=false;
if(txt)txt.textContent=`Done ${ok}/${fields.length} fail ${fail}`;
}
document.addEventListener("DOMContentLoaded",()=>{
const btn=$("#connect-btn");
const listBtn=$("#list-paired-btn");
const forgetBtn=$("#forget-btn");
if(!navigator.hid){
btn.disabled=true;btn.textContent="WebHID not supported";
setStatus("Use Chrome/Edge desktop HTTPS or http://localhost:8000");
return;
}
btn.addEventListener("click",async()=>{
if(isConnecting)return;
isConnecting=true;
btn.textContent="Searching...";
setStatus("Checking paired...");
let dev=await tryExisting();
if(dev){isConnecting=false;btn.textContent="Connected";await onConnected();return;}
setStatus("Requesting device chooser...");
dev=await requestAndOpen();
if(dev){isConnecting=false;btn.textContent="Connected";await onConnected();return;}
isConnecting=false;
btn.textContent="Connect";
setStatus("Not connected. Check List Paired, Forget All if you have ELAN touchpad paired, ensure firmware has vendor 0xFF00 page, and Linux udev rules.");
await listPaired();
});
listBtn?.addEventListener("click",listPaired);
forgetBtn?.addEventListener("click",forgetAll);
document.querySelectorAll(".tab").forEach(t=>{
t.addEventListener("click",()=>{
document.querySelectorAll(".tab").forEach(x=>x.classList.remove("active"));
t.classList.add("active");
document.querySelectorAll(".view").forEach(v=>v.classList.remove("active"));
document.getElementById("view-"+t.getAttribute("data-tab"))?.classList.add("active");
});
});
document.getElementById("profile-save")?.addEventListener("click",()=>{
const n=document.getElementById("profile-name").value.trim();
if(!n){alert("Enter name");return;}
try{saveProfile(n);refreshProfileUI();document.getElementById("profile-name").value="";}catch(e){alert(e.message);}
});
document.getElementById("profile-load")?.addEventListener("click",()=>{const n=document.getElementById("profile-active").value;if(!n)return;try{loadProfile(n);}catch(e){alert(e.message);}});
document.getElementById("profile-delete")?.addEventListener("click",()=>{const n=document.getElementById("profile-active").value;if(!n)return;if(!confirm(`Delete ${n}?`))return;deleteProfile(n);refreshProfileUI();});
document.getElementById("apply-all")?.addEventListener("click",applyAll);
document.getElementById("apply-read")?.addEventListener("click",readAll);
document.getElementById("fw-help")?.addEventListener("click",async()=>{
const txt="Hold DFU, plug, drag .bin to COBALT-DFU, unplug.";
try{await navigator.clipboard.writeText(txt);}catch{alert(txt);}
});
refreshProfileUI();
listPaired();
navigator.hid?.addEventListener("disconnect",e=>{
if(connectedDevice&&e.device===connectedDevice){setState("Disconnected");document.getElementById("apply-text").textContent="Disconnected";}
});
});
