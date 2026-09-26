import { getCurrentConfigObject, loadConfigToUI } from "./config.js";
const KEY="cobaltx_profiles_v1";const ACTIVE_KEY="cobaltx_active_profile";
function readStore(){try{const r=localStorage.getItem(KEY);if(!r)return {};return JSON.parse(r);}catch{return {};}}
function writeStore(o){localStorage.setItem(KEY,JSON.stringify(o));}
export function listProfiles(){const s=readStore();return Object.keys(s).map(n=>({name:n,data:s[n]}));}
export function saveProfile(name){
if(!name||!name.trim())throw new Error("Name required");
if(name.trim().length>32)throw new Error("max 32");
const cfg=getCurrentConfigObject();
const store=readStore();
store[name.trim()]={cfg,savedAt:Date.now()};
writeStore(store);
localStorage.setItem(ACTIVE_KEY,name.trim());
}
export function loadProfile(name){
const store=readStore();
const e=store[name];
if(!e)throw new Error("Not found");
loadConfigToUI(e.cfg||e);
localStorage.setItem(ACTIVE_KEY,name);
return e.cfg;
}
export function deleteProfile(name){
const store=readStore();
delete store[name];
writeStore(store);
if(localStorage.getItem(ACTIVE_KEY)===name)localStorage.removeItem(ACTIVE_KEY);
}
export function refreshProfileUI(){
const sel=document.getElementById("profile-active");
const listDiv=document.getElementById("profile-list");
if(!sel||!listDiv)return;
const profiles=listProfiles();
const active=localStorage.getItem(ACTIVE_KEY)||"";
sel.innerHTML="";listDiv.innerHTML="";
if(!profiles.length){sel.innerHTML='<option value="">No profiles</option>';listDiv.innerHTML='<div class="muted">No profiles</div>';return;}
profiles.forEach(p=>{
const o=document.createElement("option");o.value=p.name;o.textContent=p.name;if(p.name===active)o.selected=true;sel.appendChild(o);
const row=document.createElement("div");row.className="card";row.style.marginTop="5px";row.innerHTML=`<div class="row" style="justify-content:space-between"><span>${p.name}</span><span class="muted">${new Date(p.data.savedAt||Date.now()).toLocaleDateString()}</span></div>`;row.addEventListener("click",()=>{sel.value=p.name;});listDiv.appendChild(row);
});
if(!active&&profiles.length)sel.selectedIndex=0;
}
