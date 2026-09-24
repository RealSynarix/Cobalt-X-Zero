// js/app.js - app logic, tab switching, connect spam, apply all
import { device as hidDevice, openDevice, readField, writeField, parseResponse } from "./hid.js";
import { FIELD, FIELD_META, applyFieldToUI, gatherAllFields, loadConfigToUI, currentConfig } from "./config.js";
import { saveProfile, loadProfile, deleteProfile, refreshProfileUI, getActiveName, listProfiles } from "./profiles.js";

const VID = 0x1209;
const PID = 0xC0BA;

let connectLoop = null;
let isConnecting = false;
let connectedDevice = null;

const $ = (s)=>document.querySelector(s);

function setStatus(msg){ const el=$("#connect-status"); if(el) el.textContent = msg||""; }
function setDeviceState(msg){ const el=$("#device-state"); if(el) el.textContent = msg; }

async function tryGetExistingDevices() {
  if (!navigator.hid) return null;
  const devs = await navigator.hid.getDevices();
  const found = devs.find(d=>d.vendorId===VID && d.productId===PID);
  if (found) {
    try { await openDevice(found); connectedDevice = found; return found; } catch {}
  }
  return null;
}

async function requestDeviceOnce() {
  if (!navigator.hid) throw new Error("WebHID not supported");
  const filters = [
    { vendorId: VID, productId: PID },
    { vendorId: VID },
    { usagePage: 0xFF00 }
  ];
  const devs = await navigator.hid.requestDevice({ filters });
  if (devs && devs.length) {
    const d = devs.find(x=>x.vendorId===VID && x.productId===PID) || devs[0];
    await openDevice(d);
    connectedDevice = d;
    return d;
  }
  return null;
}

async function spamConnectFlow() {
  if (isConnecting) return;
  isConnecting = true;
  const btn = $("#connect-btn");
  btn.classList.add("searching");
  btn.textContent = "Searching... select your mouse";
  setStatus("Waiting for permission and device. If nothing appears, allow HID access.");
  try {
    // first check existing
    const existing = await tryGetExistingDevices();
    if (existing) {
      await onConnected();
      return;
    }
    // request once to trigger prompt, then loop polling
    try { await requestDeviceOnce(); if (connectedDevice) { await onConnected(); return; } } catch (e) {
      setStatus("Permission prompt was dismissed. Click again to retry.");
    }
    // poll every 800ms
    connectLoop = setInterval(async ()=>{
      const d = await tryGetExistingDevices();
      if (d) {
        clearInterval(connectLoop);
        await onConnected();
      }
    }, 800);
    // safety stop after 60s
    setTimeout(()=>{ if(connectLoop){ clearInterval(connectLoop); btn.classList.remove("searching"); btn.textContent="Connect Cobalt-X Zero"; setStatus("Search timed out. Click to try again."); isConnecting=false; } }, 60000);
  } catch (err) {
    btn.classList.remove("searching");
    btn.textContent = "Connect Cobalt-X Zero";
    setStatus(err.message||"Failed to connect");
    isConnecting = false;
  }
}

async function onConnected() {
  if (connectLoop) clearInterval(connectLoop);
  isConnecting = false;
  const btn = $("#connect-btn");
  btn.classList.remove("searching");
  setStatus("Connected. Loading settings...");
  // fade transition
  const cPage = $("#connect-page");
  const aPage = $("#app-page");
  cPage.style.transition = "opacity 0.4s ease, transform 0.4s ease";
  cPage.style.opacity = "0";
  cPage.style.transform = "translateY(-12px)";
  setTimeout(async ()=>{
    cPage.classList.add("hidden");
    aPage.classList.remove("hidden");
    aPage.style.opacity = "0";
    requestAnimationFrame(()=>{ aPage.style.transition="opacity 0.4s ease"; aPage.style.opacity="1"; });
    setDeviceState("Connected • 0x1209:0xC0BA");
    await readAllFromDevice();
    refreshProfileUI();
    bindRangeDisplays();
  }, 320);
}

async function readAllFromDevice() {
  const ids = Object.keys(FIELD_META).map(n=>parseInt(n,10)).filter(id=>{
    const m=FIELD_META[id]; return m;
  });
  const status = $("#apply-text");
  if (status) status.textContent = "Reading...";
  for (let i=0;i<ids.length;i++) {
    const fid = ids[i];
    try {
      const res = await readField(fid);
      if (res && res.payload) applyFieldToUI(fid, res.payload);
    } catch {}
    if (status) status.textContent = `Reading ${Math.round(((i+1)/ids.length)*100)}%`;
    await new Promise(r=>setTimeout(r, 18));
  }
  if (status) status.textContent = "Ready to apply";
  // identity static fields
  const vidEl = $("#f-vid"); if (vidEl) vidEl.value = "0x"+VID.toString(16).toUpperCase();
  const pidEl = $("#f-pid"); if (pidEl) pidEl.value = "0x"+PID.toString(16).toUpperCase();
}

async function applyAllSequential() {
  const btn = $("#apply-all");
  const prog = document.querySelector(".progress-fill");
  const txt = $("#apply-text");
  const fields = gatherAllFields();
  if (!fields.length) return;
  btn.disabled = true;
  let ok = 0;
  for (let i=0;i<fields.length;i++) {
    const { fieldId, payload } = fields[i];
    try {
      const res = await writeField(fieldId, payload);
      if (res.status === 0 || res.status === undefined) ok++;
    } catch {}
    const pct = Math.round(((i+1)/fields.length)*100);
    if (prog) prog.style.width = pct+"%";
    if (txt) txt.textContent = `Applying ${pct}% — ${FIELD_META[fieldId]?.name||fieldId}`;
    await new Promise(r=>setTimeout(r,60));
  }
  btn.disabled = false;
  if (txt) txt.textContent = `Applied ${ok}/${fields.length} fields`;
  setTimeout(()=>{ if (prog) prog.style.width="0%"; if (txt) txt.textContent="Ready to apply"; }, 1600);
}

function bindTabs() {
  const tabs = document.querySelectorAll(".tab");
  const views = document.querySelectorAll(".view");
  tabs.forEach(t=>{
    t.addEventListener("click", ()=>{
      tabs.forEach(x=>x.classList.remove("active"));
      t.classList.add("active");
      const id = t.getAttribute("data-tab");
      views.forEach(v=>v.classList.remove("active"));
      const target = document.getElementById("view-"+id);
      if (target) target.classList.add("active");
    });
  });
}

function bindRangeDisplays() {
  const bri = $("#f-led_bri");
  const briV = $("#f-led_bri-v");
  if (bri && briV) {
    bri.addEventListener("input", ()=>{ briV.textContent = bri.value; });
  }
  const dpi = $("#f-dpi");
  const dpiR = $("#f-dpi-r");
  if (dpi && dpiR) {
    dpi.addEventListener("input", ()=>{ dpiR.value = dpi.value; });
    dpiR.addEventListener("input", ()=>{ dpi.value = dpiR.value; });
  }
}

function bindProfiles() {
  const saveBtn = $("#profile-save");
  const nameInput = $("#profile-name");
  const loadBtn = $("#profile-load");
  const delBtn = $("#profile-delete");
  const sel = $("#profile-active");
  if (saveBtn) saveBtn.addEventListener("click", ()=>{
    try {
      const n = nameInput.value.trim();
      if (!n) { alert("Enter a profile name"); return; }
      saveProfile(n);
      refreshProfileUI();
      nameInput.value = "";
    } catch (e){ alert(e.message); }
  });
  if (loadBtn) loadBtn.addEventListener("click", ()=>{
    const n = sel.value;
    if (!n) return;
    try { loadProfile(n); } catch (e){ alert(e.message); }
  });
  if (delBtn) delBtn.addEventListener("click", ()=>{
    const n = sel.value;
    if (!n) return;
    if (!confirm(`Delete profile "${n}"?`)) return;
    deleteProfile(n);
    refreshProfileUI();
  });
}

function bindApply() {
  $("#apply-all")?.addEventListener("click", applyAllSequential);
  $("#apply-read")?.addEventListener("click", readAllFromDevice);
}

function bindFlash() {
  const help = document.getElementById("fw-help");
  if (help) help.addEventListener("click", async ()=>{
    const txt = "Hold DFU button + plug, copy .bin to COBALT-DFU drive, unplug and replug.";
    try { await navigator.clipboard.writeText(txt); help.textContent="Copied"; setTimeout(()=>help.textContent="Copy instructions", 1200); } catch { alert(txt); }
  });
}

// init
document.addEventListener("DOMContentLoaded", ()=>{
  const btn = $("#connect-btn");
  if (!btn) return;
  if (!navigator.hid) {
    btn.disabled = true;
    btn.textContent = "WebHID not supported";
    setStatus("Use Chrome or Edge 89+ on desktop over HTTPS. GitHub Pages HTTPS is fine.");
    return;
  }
  btn.addEventListener("click", spamConnectFlow);
  bindTabs();
  bindProfiles();
  bindApply();
  bindFlash();
  refreshProfileUI();
  // listen disconnect
  navigator.hid?.addEventListener("disconnect", (e)=>{
    if (connectedDevice && e.device === connectedDevice) {
      setDeviceState("Disconnected");
      const txt = document.getElementById("apply-text");
      if (txt) txt.textContent = "Device disconnected";
    }
  });
});
