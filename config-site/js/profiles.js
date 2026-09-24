// js/profiles.js - localStorage profiles
import { getCurrentConfigObject, loadConfigToUI } from "./config.js";

const KEY = "cobaltx_profiles_v1";
const ACTIVE_KEY = "cobaltx_active_profile";

function readStore() {
  try {
    const raw = localStorage.getItem(KEY);
    if (!raw) return {};
    return JSON.parse(raw);
  } catch { return {}; }
}
function writeStore(obj) {
  localStorage.setItem(KEY, JSON.stringify(obj));
}

export function listProfiles() {
  const store = readStore();
  return Object.keys(store).map(name=>({ name, data: store[name] }));
}
export function saveProfile(name) {
  if (!name || !name.trim()) throw new Error("Name required");
  const cfg = getCurrentConfigObject();
  const store = readStore();
  store[name.trim()] = { cfg, savedAt: Date.now() };
  writeStore(store);
  localStorage.setItem(ACTIVE_KEY, name.trim());
}
export function loadProfile(name) {
  const store = readStore();
  const entry = store[name];
  if (!entry) throw new Error("Profile not found");
  const cfg = entry.cfg || entry;
  loadConfigToUI(cfg);
  localStorage.setItem(ACTIVE_KEY, name);
  return cfg;
}
export function deleteProfile(name) {
  const store = readStore();
  delete store[name];
  writeStore(store);
  if (localStorage.getItem(ACTIVE_KEY)===name) localStorage.removeItem(ACTIVE_KEY);
}
export function getActiveName() {
  return localStorage.getItem(ACTIVE_KEY) || "";
}
export function refreshProfileUI() {
  const sel = document.getElementById("profile-active");
  const listDiv = document.getElementById("profile-list");
  if (!sel || !listDiv) return;
  const profiles = listProfiles();
  const active = getActiveName();
  sel.innerHTML = "";
  listDiv.innerHTML = "";
  if (profiles.length===0) {
    sel.innerHTML = '<option value="">No profiles yet</option>';
    listDiv.innerHTML = '<div class="muted">No profiles saved. Adjust settings then save.</div>';
    return;
  }
  profiles.forEach(p=>{
    const o = document.createElement("option");
    o.value = p.name;
    o.textContent = p.name;
    if (p.name===active) o.selected = true;
    sel.appendChild(o);
    const row = document.createElement("div");
    row.className = "profile-item";
    row.innerHTML = `<span>${p.name}</span><span class="muted small">${new Date(p.data.savedAt||Date.now()).toLocaleDateString()}</span>`;
    row.addEventListener("click", ()=>{ sel.value = p.name; });
    listDiv.appendChild(row);
  });
  if (!active && profiles.length) sel.selectedIndex = 0;
}
