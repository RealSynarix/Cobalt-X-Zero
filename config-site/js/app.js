import { device, openDevice, readField, writeField, ping } from "./hid.js";
import { FIELD, FIELD_META, applyFieldToUI, gatherAllFields, loadConfigToUI } from "./config.js";
import { saveProfile, loadProfile, deleteProfile, refreshProfileUI } from "./profiles.js";
const VID=0x1209;const PID=0xC0BA;
let connectLoop=null;let isConnecting=false;let connectedDevice=null;
const $=s=>document.querySelector(s);
function setStatus(m){const el=$("#connect-status");if(el)el.textContent=m||"";}
function setDeviceState(m){const el=$("#device-state");if(el)el.textContent=m;}
async function tryOpen(d){
try{
if(!d.opened)await d.open();
const { openDevice: od } = await import("./hid.js");
await od(d);
connectedDevice=d;
return true;
}catch(e){
setStatus(`Open failed: ${e.message}. Paired but busy. Close other apps, click Connect again.`);
return false;
}
}
async function tryExisting(){
if(!navigator.hid)return null;
try{
const devs=await navigator.hid.getDevices();
const found=devs.find(x=>x.vendorId===VID&&x.productId===PID);
if(found){
const ok=await tryOpen(found);
if(ok)return found;
}
}catch(e){setStatus(`getDevices error: ${e.message}`);}
return null;
}
async function requestAndOpen(){
if(!navigator.hid)throw new Error("WebHID not supported");
const filters=[{vendorId:VID,productId:PID},{vendorId:VID},{usagePage:0xFF00}];
try{
const devs=await navigator.hid.requestDevice({filters});
if(!devs||!devs.length){setStatus("Prompt dismissed. Click Connect again.");return null;}
const d=devs.find(x=>x.vendorId===VID&&x.productId===PID)||devs[0];
const ok=await tryOpen(d);
if(ok)return d;
return null;
}catch(e){
if(e.name==="NotAllowedError"||e.name==="AbortError"){setStatus("Prompt dismissed. Click Connect again.");}
else{setStatus(`Request failed: ${e.message}`);}
return null;
}
}
async function listPaired(){
if(!navigator.hid)return;
try{
const devs=await navigator.hid.getDevices();
const el=$("#paired-list");
if(!el)return;
if(!devs.length){el.textContent="No paired";return;}
el.textContent=devs.map(d=>`${d.productName||'dev'} ${d.vendorId.toString(16)}:${d.productId.toString(16)}${d.opened?' opened':''}`).join(' | ');
}catch(e){$("#paired-list").textContent=e.message;}
}
async function onConnected(){
if(connectLoop)clearInterval(connectLoop);
isConnecting=false;
$("#connect-btn").textContent="Connected";
setStatus("Reading...");
$("#connect-page").classList.add("hidden");
$("#app-page").classList.remove("hidden");
setDeviceState("Connected 0x1209:0xC0BA");
try{const p=await ping();setStatus(`Ping OK status=${p.status}`);}catch{setStatus("Ping failed, reading anyway");}
await readAll();
refreshProfileUI();
}
async function readAll(){
const ids=Object.keys(FIELD_META).map(n=>parseInt(n,10));
const status=$("#apply-text");
if(status)status.textContent="Reading...";
for(let i=0;i<ids.length;i++){
const fid=ids[i];
try{
const res=await readField(fid);
if(res&&res.status===0&&res.payload)applyFieldToUI(fid,res.payload);
}catch{}
if(status)status.textContent=`Reading ${Math.round(((i+1)/ids.length)*100)}%`;
await new Promise(r=>setTimeout(r,18));
}
if(status)status.textContent="Ready";
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
await new Promise(r=>setTimeout(r,60));
}
btn.disabled=false;
if(txt)txt.textContent=`Done ${ok}/${fields.length} fail ${fail}`;
setTimeout(()=>{if(prog)prog.style.width="0%";if(txt)txt.textContent="Ready";},1500);
}
function bindTabs(){
document.querySelectorAll(".tab").forEach(t=>{
t.addEventListener("click",()=>{
document.querySelectorAll(".tab").forEach(x=>x.classList.remove("active"));
t.classList.add("active");
const id=t.getAttribute("data-tab");
document.querySelectorAll(".view").forEach(v=>v.classList.remove("active"));
document.getElementById("view-"+id)?.classList.add("active");
});
});
}
function bindProfiles(){
$("#profile-save")?.addEventListener("click",()=>{
const n=$("#profile-name").value.trim();
if(!n){alert("Enter name");return;}
try{saveProfile(n);refreshProfileUI();$("#profile-name").value="";}catch(e){alert(e.message);}
});
$("#profile-load")?.addEventListener("click",()=>{const n=$("#profile-active").value;if(!n)return;try{loadProfile(n);}catch(e){alert(e.message);}});
$("#profile-delete")?.addEventListener("click",()=>{const n=$("#profile-active").value;if(!n)return;if(!confirm(`Delete "${n}"?`))return;deleteProfile(n);refreshProfileUI();});
}
document.addEventListener("DOMContentLoaded",()=>{
const btn=$("#connect-btn");
const listBtn=$("#list-paired-btn");
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
if(dev){await onConnected();return;}
setStatus("Requesting device...");
dev=await requestAndOpen();
if(dev){await onConnected();return;}
connectLoop=setInterval(async()=>{
const d=await tryExisting();
if(d){clearInterval(connectLoop);await onConnected();}
},800);
setTimeout(()=>{clearInterval(connectLoop);isConnecting=false;btn.textContent="Connect";setStatus("Timed out. Click again.");},30000);
});
listBtn?.addEventListener("click",listPaired);
document.querySelectorAll(".tab").forEach(t=>t.addEventListener("click",()=>{}));
bindTabs();bindProfiles();
$("#apply-all")?.addEventListener("click",applyAll);
$("#apply-read")?.addEventListener("click",readAll);
$("#fw-help")?.addEventListener("click",async()=>{
const txt="Hold DFU, plug, drag .bin to COBALT-DFU, unplug.";
try{await navigator.clipboard.writeText(txt);const b=$("#fw-help");b.textContent="Copied";setTimeout(()=>b.textContent="Copy steps",1200);}catch{alert(txt);}
});
refreshProfileUI();listPaired();
navigator.hid?.addEventListener("disconnect",e=>{
if(connectedDevice&&e.device===connectedDevice){setDeviceState("Disconnected");$("#apply-text").textContent="Disconnected";}
});
});
