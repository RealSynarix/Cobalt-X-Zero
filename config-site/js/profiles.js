import { getCurrentConfigObject, loadConfigToUI } from "./config.js";
const KEY="cobaltx_profiles_v1";const ACTIVE="cobaltx_active_profile";
function readStore(){try{const r=localStorage.getItem(KEY);if(!r)return {};return JSON.parse(r);}catch{return {};}}
function writeStore(o){localStorage.setItem(KEY,JSON.stringify(o));}
export function saveProfile(name){
if(!name||!name.trim())throw new Error("Name required");
if(name.trim().length>32)throw new Error("max 32");
const cfg=getCurrentConfigObject();
const s=readStore();
s[name.trim()]={cfg,savedAt:Date.now()};
writeStore(s);
localStorage.setItem(ACTIVE,name.trim());
}
export function loadProfile(name){
const s=readStore();
const e=s[name];
if(!e)throw new Error("Not found");
loadConfigToUI(e.cfg||e);
localStorage.setItem(ACTIVE,name);
}
export function deleteProfile(name){
const s=readStore();
delete s[name];
writeStore(s);
if(localStorage.getItem(ACTIVE)===name)localStorage.removeItem(ACTIVE);
}
export function refreshProfileUI(){
const sel=document.getElementById("profile-active");
const list=document.getElementById("profile-list");
if(!sel||!list)return;
const profiles=Object.keys(readStore()).map(n=>({name:n,data:readStore()[n]}));
const active=localStorage.getItem(ACTIVE)||"";
sel.innerHTML="";list.innerHTML="";
if(!profiles.length){sel.innerHTML='<option value="">No profiles</option>';list.innerHTML='<div class="muted">No profiles</div>';return;}
profiles.forEach(p=>{
const o=document.createElement("option");o.value=p.name;o.textContent=p.name;if(p.name===active)o.selected=true;sel.appendChild(o);
const row=document.createElement("div");row.className="card2";row.style.marginTop="4px";row.innerHTML=`<div class="row" style="justify-content:space-between"><span>${p.name}</span><span class="muted">${new Date(p.data.savedAt||Date.now()).toLocaleDateString()}</span></div>`;row.addEventListener("click",()=>{sel.value=p.name;});list.appendChild(row);
});
}
