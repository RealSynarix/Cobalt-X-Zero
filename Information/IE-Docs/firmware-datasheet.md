
- Additional encoder steps while not IDLE increment `pending_clicks`.
- The target button bit appears in the HID report exactly like a physical press – no long‑press or merging.

### 6.4 MMB Behaviour in HyprX

- Physical middle button press does **not** produce a middle click.
- Instead, MMB **toggles the scroll‑click target** between LMB and RMB.
- Toggling occurs on the **release edge** of MMB.

### 6.5 LMB and RMB

- Left and right physical buttons **always retain their normal independent click functions**, even in HyprX.

### 6.6 State Volatility

- **HyprX mode and click target are never saved to flash.**
- Every power‑cycle starts in **Normal mode** with the click target set to **LMB**.

---

## 7. MAC2 Button

- Ships **inert** – no HID reports generated.
- Fully **user‑programmable** via `macro2.json` on the MSC virtual drive.
- If a valid `macro2.json` is present at boot, the firmware arms MAC2; otherwise the button remains dormant.
- The exact schema for `macro2.json` will be defined by the project owner.

---

## 8. MSC Virtual Drive

The configuration boot exposes a mass storage device over USB.

### 8.1 Enumerated Parameters

- **Advertised capacity:** 67 MB (SCSI READ CAPACITY).
- **Actual backing store:** only a few kilobytes of RAM – enough for `config.ini` and `macro2.json`.

### 8.2 Filesystem Structure

- A FAT filesystem (boot sector, FAT, root directory) is **generated dynamically in RAM** at enumeration, rebuilt from flash‑stored config at every boot.
- Two files always appear: `CONFIG.INI` and `MACRO2.JSON` (8.3 format).

### 8.3 Read/Write Handling

- **Reads** to sectors belonging to the two files return data from the in‑RAM buffers.
- **Writes** to those sectors update the RAM buffers and trigger a flash write of the new content.
- **All writes to any other sector** (the “empty” 67 MB, FAT region, directory, format attempts) are **silently accepted and discarded**. The SCSI command returns success but no data is stored, preventing host errors or filesystem corruption.
- **Renames, deletes, and reformats** are similarly acknowledged but ignored.

### 8.4 Safeguard Rationale

Because the host believes it has a full 67 MB drive, the firmware must gracefully handle any operation that assumes real storage. Only writes to the two configuration files are persisted.

---

## 9. Flash Budget & Image Management

- **Total flash:** 128 KB.
- The unified image must contain HyprBoot + HID + MSC.
- Before any erasure, all three coexist:  
  `size(HyprBoot) + size(HID) + size(MSC) ≤ 128 KB` with a safe margin.
- After selection, the active image erases the other two and may reuse the freed space.
- **Continuous tracking** of component sizes is mandatory throughout development.

---

## 10. Host Development Environment & Flashing

- **Toolchain:** ARM GCC (bare‑metal), any compatible IDE (Keil, IAR, VS Code, etc.).
- **Build output:** A single binary (`.bin` or `.hex`) containing the unified production image.
- **Flashing procedure:**
  1. Disconnect the mouse from USB (if connected).
  2. Press and hold the **DFU button** while connecting the USB cable.
  3. The MCU enters its built‑in DFU bootloader.
  4. Use a standard tool (e.g., `dfu-util`, STM32CubeProgrammer) to flash the firmware.
  5. After flashing, the device resets and runs the newly programmed image.
- The MSC virtual drive **cannot** be used for firmware updates; it is solely for configuration files.

---

## 11. Configuration Files (Placeholder)

The schemas for `config.ini` and `macro2.json` will be supplied by the project owner. The firmware must parse them robustly and treat any missing or invalid entries as defaults. Final details are outside the scope of this datasheet but will be provided before implementation.

---

*This datasheet is a template; the project owner’s specifications always take precedence.*
