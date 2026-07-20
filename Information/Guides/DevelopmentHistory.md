# Developer Log: From Nothing To Something Great

### x/2/26: The Vision & The Pivot
Initially, I looked into the RP2040 MCU, but after community analysis and help from Reddit, I realized it was too complex to trace for a compact mouse footprint. I pivoted to the STM32G4 series for its speed and compact GPIO layout.

The PMW3360 sensor was a major hurdle. It required an external regulator to drop 5V to 3.3V to avoid frying the IC, and it took days of studying documentation to understand the SPI pin layout.

### x/2/26: Prototype V1 - "The Learning Tax"
I ordered my first PCB from JLCPCB and realized my mistakes only after it arrived:
* **Footprint Mismatch:** I used 4-pin button footprints for 2-pin buttons.
* **Mirroring Error:** I mirrored the pins on the daughterboard and mainboard connectors, making them impossible to connect.
* **Mechanical Errors:** The sizing for the scroll wheel housing (edge.cuts) was off.
* **Boot Failure:** I had no way to enter DFU mode easily, leading to hours of frustration trying to connect to the device via boot pins to no avail.

### 28/2/26: Revision V2 - The Flow-State Optimization
I completely re-evaluated and rebuilt the design based on the V1 failures. Rather than taking days or weeks, I finished the entire schematic rework and PCB re-routing in a single 7-hour session. 

I entered a complete "flow-state" so I had no breaks, was hyper-focused, and had total recall of every niche technical detail. By the end of the afternoon, I had executed:
* **Hardware Fixes:** Corrected all button footprints and fixed the mirrored daughterboard connectors.
* **New Features:** Added a dedicated DFU button and test pads for clock/firmware reversion.
* **Weight Reduction:** Optimized for 1.2mm PCB thickness. Also removed the daugherboard for less space and no cables.

### 7/3/26: Final Assembly & Post-Processing (Plans)
Currently I am waiting for the board to arrive and flash the firmware onto it. once it arrives in a few days and I do that, I will focus on the model of the 3D-printed shell in Black PETG using an Elegoo Neptune 4 Pro, and modeled via TinkerCAD. I am using a sanding process and other post-processing to remove layer lines and achieve a matte finish that looks like a professional injection-molded product for the final competition entry.

### 22/3/26: Board Arrived & Firmware Works
After I recieved the PCB, I decided I would not solder the parts myself as I'm not good at it and may join connections or damage components. I went to a shop near me and got it soldered for 20 bucks! I was expecting alot more but thats great! Anyway, once I got the PCB, I could not for the life of me get the firmware right! I'll be upfront, I am using AI to build the firmware, becasue lets be real, AI is a tool and why would I not use a tool to save a year in coding? Also, I am very good at prompting AI so that is how I have been able todo all of this. Anyway, It took me so long to get the firmware right, but I ended up using the Arduino IDE and set my board to the right one and with some other tweaks and stuff, It worked. I used Claude Sonnet and gave a like 500 word prompt and told it exactly what todo, and after a few itterations and fixes, I got my device recognised as a HID over USB! It is 2am as of writing this so Im not doing any more firmware dev, but this is massive progress.

### 4/5/26: Firmware Update & Bad News
It's been awhile, and honestly I would of expected this project to be done by now, but with school work and personal events, its been a very big challenge and i ahve but a hold on this project. However, during the time from the last devlog to now, I have made significant changes and found a critical flaw. First off, I have mostly finished the firmware, with the exception of the sensor logic and vDrive, and I also am planning to create a bootloader and have 2 firmware modules for HID and MSC. The issue I know face and will tank the time for this proejct (and budget), is that for some reason I stupidly put the power pin (VDD) for the sensor module to ground, meaning it doesnt get power. Not only that, but the LEDs i chose are actually not the form factor I want, so now I must redesign the PCB, AND redesign the schematic regarding LEDs, AND finish the firmware, AND finish the 3D model. Honestly I'm not too happy currently and I'm not sure if I can get it done, but for now I will rest and hope that this goes smoothly, and I can develop and produce the optimal gaming mice the community deserves.
