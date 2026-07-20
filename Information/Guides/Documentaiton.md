# Project Documentation: Cobalt-X Zero

## 1. The Concept
I was tinkering with some hardware at my homelab, thinking about what I could create that would solve a personal problem and potentially help others. I looked at my desktop mouse and realized I wanted something more: silent clicks, a high-quality scroll wheel, a custom shell, and hardware-level macros. 

At 14 years and 9 months, I had the technical drive, but I had never designed a PCB or soldered components before. I was a Linux and programming enthusiast, but I wanted to take my skills into the physical world. After just a few months of self-teaching KiCad, electrical schematics, and trace routing, and 3D design, the Cobalt-X Zero was born.

## 2. PCB Architecture & Details
The goal was a "minimal and polished" approach. I chose components based on affordability, ease of routing, specs, and physical footprint:
* **MCU:** STM32G431KBT6 (170MHz ARM Cortex-M4) – Chosen for its massive 170mhz clock and ease of routing.
* **Sensor:** PixArt PMW3360DM-T2QU – A high-performance optical sensor that is very well doccumented that provides industry-leading tracking at 12,000 DPI.
* **PCB Density:** Designed on a 1.2mm board to reduce weight by ~25% compared to standard 1.6mm boards while maintaining structural integrity. Also the edge.cuts has been made to be symetrical for a clean appearence.
* **Traces/Routing:** Fully hand traced PCB with diff pairs for the data lines and diff widths for power, data, and general traces. Also the board has a ground plane for less traces and a better signal integrity.

### Advanced Signal Processing:
To achieve "Zero-Latency," the firmware utilizes:
* **Internal 32kHz Oversampling:** The MCU polls the sensor at 32,000Hz internally, allowing for advanced jitter filtering before data is sent.
* **DMA (Direct Memory Access):** I implemented DMA to handle sensor data transfer, freeing up the MCU for packet optimization and reducing interrupt latency.
* **Debounce Algorithem:** I implemented this via the firmware to ensure the click packets for buttons have sub-ms latency and have implemented a function to prevent double clicks/accidental clicks by waiting a few nanoseconds before allowing another click.
* **Predictive Packet Readiness:** The firmware prepares HID packets in advance of the USB/OS request. When the Host asks for data, it is delivered instantly from the buffer.

## 3. The "Zero-Software" Solution & VFS
I daily drive Arch Linux and hate closed-source, proprietary, or bloated software. Most modern mice require heavy Windows-specific drivers/config-software for basic configuration.

The Cobalt-X Zero mounts as a virtual mass storage device (67MB for no particular reason) when the macro button is held during plug-in. This allows users to edit a `config.ini` file directly to change things like (this and more settings):
* **DPI & Polling Rate**
* **LOD (Lift Off Distance)**
* **Device Name (Default is Cobalt-X Zero)**


## 4. HyprX Macro System & Aesthetics
The mouse features a sleek matte black finish with a neon blue underglow (hence the name including cobalt). It incorporates a dedicat button located jsut under the scroll wheel (same area as a DPI button) to toggle the **HyprX** system:
* **Visual Feedback:** Lighting switches from "Neon Blue" to "Blazing Red" when active.
* **Rapid Action:** Maps the TTC Gold 24-step encoder to rapid-fire LMB/RMB clicks.
* **Macro Click Swap:** If the MMB is pressed while in the macro mode, scrolling wil switch from clicking the LMB to the RMB and vise versa.
* **Hotswap USB-C:** Unlike most wired mice, the cable is not soldered. This reduces e-waste and allows users to use custom paracord cables or any other usb-c cable for a "wireless" feel. This also cuts costs and time by not neding to solder wires and assuming a usb port, when it can be now put on a usb or usb-c port (and potentialy other interfaces aswell).

