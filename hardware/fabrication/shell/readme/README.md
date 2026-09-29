# 3D Models - Shell

Everything needed to print shell plus slicer settings and post-processing.

| File | What it is |
| --- | --- |
| `../Bottom-Plate.stl` | Bottom plate with ribs/pockets that prevent wobble and locate PCB |
| `../Top-Shell.stl` | Top shell |
| `../Scroll-Wheel.stl` | Custom wheel for EC10 |
| `../Side-Buttons.stl` | Side buttons |
| `../Top-Plungers.stl` | Plungers and studs |
| `../PCB-Holder.stl` | Small clip that goes on top of bottom shell near MCU hole, holds board down so it won't lift |
| `settings.txt` | Slicer settings |
| `post-processing.txt` | Cleaning and fit |
| `README.md` | This file |

### PCB holder and bottom shell

Bottom shell has integrated ribs and pockets that hold PCB and stop wobble side to side. PCB holder is separate small clip on top of bottom shell that presses board down so it won't lift up and down. Test upside down, board should not move.

### Plunger studs

Plungers include little ~2mm blocks that clip onto underside of top shell into 2 holes under 2 large plungers. Those are what clicks onto mechanical switches for LMB/RMB. Shortest stud is RMB, longest is LMB. Push into slots clearly visible. May be deprecated later but rev 3 needs them.

### How I modelled these

I'm extremely happy with how these turned out. I practically mastered TinkerCAD just doing this project alone. I didn't use Blender or Fusion. I used specific shaped cutouts, the sketch tool to get exact profiles, and like 100ish hole objects layered on the baseplate to get curves and clearances right. It sounds messy but it works and final fit is tight.

### Print info

| Setting | Value |
| --- | --- |
| Material | PETG |
| Layer | 0.25mm first 0.35mm |
| Infill | 9% |
| Support | Tree auto |
| Brim | None |
| Fuzzy | Off |

Follow settings.txt and post-processing.txt, then docs/guides/assembly.md. Flash from firmware/build/ before closing top shell. Test at https://realsynarix.github.io/Cobalt-X-Zero/.