# Assembly Guide

How to put Cobalt-X Zero together from PCB and printed parts. Flash firmware before you close the top shell, you can't reach DFU button after.

### What you need

**PCB side:**
- Board fabricated from `hardware/fabrication/pcb/Cobalt-X_Zero_rev-3.zip`
- All parts from BOM soldered
- USB cable with Type-C on one end, data cable not power-only

**Shell side:**
- Printed parts: Bottom-Plate, Top-Shell, Scroll-Wheel, Side-Buttons, Top-Plungers (includes studs), PCB-Holder
- 4x small screws (the masruements are around 4mm length, 2.15mm for the top IIRC)
- 4x 0.75mm PTFE skates
- Scroll wheel, side buttons
- Grip tape
- Sensor lens

Make sure prints are post-processed per `hardware/fabrication/shell/readme/post-processing.txt`.

### Steps

| Step | Action |
| --- | --- |
| 1 | **Lens** - Press lens firmly into bottom plate until even and it snaps. Check orientation. If PCB later won't sit snug, lens is 180 degrees off, flip it. |
| 2 | **Scroll wheel** - Push scroll wheel firmly into rotary encoder on PCB. You may need to flex encoder to side slightly, it won't break or bend. |
| 3 | **PCB into bottom** - Slot PCB into bottom shell. It should fit very tightly once slid in. Align with scroll wheel, there are alignment helpers in bottom plate that stop wobble side to side. |
| 4 | **PCB holder** - PCB holder is a small clip that goes on top of bottom shell and holds board onto shell so it won't lift up and down. Bottom shell has features that hold it and prevent wobbling. Push holder down onto hole on side of bottom plate near MCU. It should click and hold MCU. Test by flipping upside down, board should not move. |
| 5 | **Studs** - Studs are little ~2mm blocks that clip onto underside of top shell. They are what clicks onto mechanical switches for LMB/RMB. Get top shell and studs and push them into slots, clearly visible underside of 2 large plungers, 2 holes. Shortest stud is RMB, longest is LMB. They may be deprecated later, but rev 3 needs them. |
| 6 | **Flash firmware** - MUST flash before top shell. You cannot access DFU button after top shell assembled. Source in `/firmware/source` for dev, binary at `/firmware/cobalt-x_zero.bin` for consumer. See `firmware.md`. |
| 7 | **Close shells** - Flip top shell right way, bottom shell with all its stuff in other hand. Tilt bottom so USB/front is tilted up and back slightly down, then align horizontally and vertically and push forward, leaning down so it evens out. It should clip in, then move slightly to snug it more. |
| 8 | **Screws** - Flip over and screw in 4 holes. You may need to move top shell slightly to line up. |
| 9 | **Tighten** - Screws must be very tight. Looking from side, they should not extrude, they should be flush as part of bottom shell surface. |
| 10 | **Skates** - Stick PTFE skates right on top of screws, covering them. |
| 11 | **Grip tape** - Cut to how you like and stick on LMB big plunger, RMB big plunger, under side buttons, and other side of top shell side button area, ring finger rest. |
| 12 | **Done** - Plug cable into mouse then PC. Should enumerate as Synarix Cobalt-X Zero. Open config site. |

### Notes

| Point | Detail |
| --- | --- |
| Lens | If PCB doesn't sit flat, lens is backwards |
| PCB holder | Small clip on top of bottom shell, holds down, test upside down |
| Bottom shell | Has standoffs to stop wobble |
| Studs | ~2mm blocks under top shell, shortest RMB longest LMB |
| DFU | Only accessible before top shell |
| Screws | Must be flush or skates won't sit flat |
