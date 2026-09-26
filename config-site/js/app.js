import { openDevice, readField, writeField, ping } from "./hid.js";
import { FIELD, FIELD_META, applyFieldToUI, gatherAllFields, loadConfigToUI, currentConfig } from "./config.js";
import { saveProfile, loadProfile, deleteProfile, refreshProfileUI } from "./profiles.js";
const VID=0x1209;const PID=0xC0BA;
let connectLoop=null;let isConnecting=false;let connectedDevice=null;
const $=s=>document.querySelector(s);
function setStatus(m){const el=$("#connect-status");if(el)el.textContent=m||"";}
function setDeviceState(m){const el=$("#device-state");if(el)el.textContent=m;}
async function tryGetExistingDevices(){
if(!navigator.hid)return null;
const devs=await navigator.hid.getDevices();
const found=devs.find(d=>d.vendorId===VID&&d.productId===PID);
if(found){try{await openDevice(found);connectedDevice=found;return found;}catch{}}
return null;
}
async function listPaired(){
if(!navigator.hid)return;
const devs=await navigator.hid.getDevices();
const el=$("#paired-list");
if(!el)return;
if(!devs.length){el.textContent="No paired";return;}
el.textContent=devs.map(d=>`${d.productName||'Unknown'} ${d.vendorId.toString(16)}:${d.productId.toString(16)} ${d.opened?'opened':''}`).join(' | ');
}
async function requestDeviceOnce(){
if(!navigator.hid)throw new Error("WebHID not supported");
const filters=[{vendorId:VID,productId:PID},{vendorId:VID},{usagePage:0xFF00}];
const devs=await navigator.hid.requestDevice({filters});
if(devs&&devs.length){
const d=devs.find(x=>x.vendorId===VID&&x.productId===PID)||devs[0];
await openDevice(d);
connectedDevice=d;
return d;
}
return null;
}
async function spamConnectFlow(){
if(isConnecting)return;
isConnecting=true;
const btn=$("#connect-btn");
btn.textContent="Searching... select your mouse";
setStatus("Waiting for permission. If nothing appears, allow HID access. Must be HTTPS or localhost:8000");
try{
const existing=await tryGetExistingDevices();
if(existing){await onConnected();return;}
try{await requestDeviceOnce();if(connectedDevice){await onConnected();return;}}catch(e){setStatus("Prompt dismissed. Click again.");}
connectLoop=setInterval(async()=>{
const d=await tryGetExistingDevices();
if(d){clearInterval(connectLoop);await onConnected();}
},800);
setTimeout(()=>{if(connectLoop){clearInterval(connectLoop);btn.textContent="Connect Cobalt-X Zero";setStatus("Timed out. Click again.");isConnecting=false;}},60000);
}catch(err){
btn.textContent="Connect Cobalt-X Zero";
setStatus(err.message||"Failed");
isConnecting=false;
}
}
async function onConnected(){
if(connectLoop)clearInterval(connectLoop);
isConnecting=false;
const btn=$("#connect-btn");
btn.textContent="Connected";
setStatus("Connected. Reading...");
const cPage=$("#connect-page");
const aPage=$("#app-page");
cPage.classList.add("hidden");
aPage.classList.remove("hidden");
setDeviceState("Connected 0x1209:0xC0BA Report2");
try{const p=await ping();setStatus(`Ping OK status=${p.status}`);}catch{setStatus("Ping failed, trying read");}
await readAllFromDevice();
refreshProfileUI();
bindRangeDisplays();
}
async function readAllFromDevice(){
const ids=Object.keys(FIELD_META).map(n=>parseInt(n,10));
const status=$("#apply-text");
if(status)status.textContent="Reading...";
for(let i=0;i<ids.length;i++){
const fid=ids[i];
try{
const res=await readField(fid);
if(res&&res.status===0&&res.payload)applyFieldToUI(fid,res.payload);
}catch(e){}
if(status)status.textContent=`Reading ${Math.round(((i+1)/ids.length)*100)}%`;
await new Promise(r=>setTimeout(r,20));
}
if(status)status.textContent="Ready";
const vidEl=$("#f-vid");if(vidEl)vidEl.value="0x"+VID.toString(16).toUpperCase();
const pidEl=$("#f-pid");if(pidEl)pidEl.value="0x"+PID.toString(16).toUpperCase();
}
async function applyAllSequential(){
const btn=$("#apply-all");
const prog=document.querySelector(".progress-fill");
const txt=$("#apply-text");
let fields;
try{fields=gatherAllFields();}catch(e){alert(e.message);return;}
if(!fields.length)return;
btn.disabled=true;
let ok=0;let fail=0;
for(let i=0;i<fields.length;i++){
const {fieldId,payload}=fields[i];
try{
const res=await writeField(fieldId,payload);
if(res.status===0)ok++;else fail++;
}catch{fail++;}
const pct=Math.round(((i+1)/fields.length)*100);
if(prog)prog.style.width=pct+"%";
if(txt)txt.textContent=`Applying ${pct}% ${FIELD_META[fieldId]?.name||fieldId} ok=${ok} fail=${fail}`;
await new Promise(r=>setTimeout(r,70));
}
btn.disabled=false;
if(txt)txt.textContent=`Applied ${ok}/${fields.length} fail ${fail}`;
setTimeout(()=>{if(prog)prog.style.width="0%";if(txt)txt.textContent="Ready";},1800);
}
function bindTabs(){
const tabs=document.querySelectorAll(".tab");
const views=document.querySelectorAll(".view");
tabs.forEach(t=>{
t.addEventListener("click",()=>{
tabs.forEach(x=>x.classList.remove("active"));
t.classList.add("active");
const id=t.getAttribute("data-tab");
views.forEach(v=>v.classList.remove("active"));
const target=document.getElementById("view-"+id);
if(target)target.classList.add("active");
});
});
}
function bindRangeDisplays(){
const bri=$("#f-led_bri");const briV=$("#f-led_bri-v");
if(bri&&briV){bri.addEventListener("input",()=>{briV.textContent=bri.value;});}
}
function bindProfiles(){
const saveBtn=$("#profile-save");const nameInput=$("#profile-name");const loadBtn=$("#profile-load");const delBtn=$("#profile-delete");const sel=$("#profile-active");
if(saveBtn)saveBtn.addEventListener("click",()=>{
try{
const n=nameInput.value.trim();
if(!n){alert("Enter name");return;}
saveProfile(n);refreshProfileUI();nameInput.value="";
}catch(e){alert(e.message);}
});
if(loadBtn)loadBtn.addEventListener("click",()=>{
const n=sel.value;if(!n)return;try{loadProfile(n);}catch(e){alert(e.message);}});
if(delBtn)delBtn.addEventListener("click",()=>{
const n=sel.value;if(!n)return;if(!confirm(`Delete "${n}"?`))return;deleteProfile(n);refreshProfileUI();
});
}
function bindApply(){
$("#apply-all")?.addEventListener("click",applyAllSequential);
$("#apply-read")?.addEventListener("click",readAllFromDevice);
}
function bindFlash(){
const help=document.getElementById("fw-help");
if(help)help.addEventListener("click",async()=>{
const txt="Hold DFU button + plug, copy .bin to COBALT-DFU drive, unplug replug.";
try{await navigator.clipboard.writeText(txt);help.textContent="Copied";setTimeout(()=>help.textContent="Copy steps",1200);}catch{alert(txt);}
});
}
document.addEventListener("DOMContentLoaded",()=>{
const btn=$("#connect-btn");
const listBtn=$("#list-paired-btn");
if(!navigator.hid){
if(btn){btn.disabled=true;btn.textContent="WebHID not supported";}
setStatus("Use Chrome/Edge 89+ desktop HTTPS or http://localhost:8000");
return;
}
if(btn)btn.addEventListener("click",spamConnectFlow);
if(listBtn)listBtn.addEventListener("click",listPaired);
bindTabs();bindProfiles();bindApply();bindFlash();refreshProfileUI();listPaired();
navigator.hid?.addEventListener("disconnect",e=>{
if(connectedDevice&&e.device===connectedDevice){
setDeviceState("Disconnected");
const txt=document.getElementById("apply-text");
if(txt)txt.textContent="Disconnected";
}
});
});
