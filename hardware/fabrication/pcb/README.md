# PCB Fabrication

Files to manufacture and edit PCB.

| File | Purpose |
| --- | --- |
| `Cobalt-X_Zero.kicad_sch` | Schematic |
| `Cobalt-X_Zero.kicad_pcb` | Layout |
| `Cobalt-X_Zero_rev-3.zip` | Gerbers |
| `Cobalt-X_Zero_rev-3_bom.csv` | BOM |
| `positions.csv` | Pick and place |
| `netlist.ipc` | Netlist |

### Order

Upload zip to fab, 4-layer 1.6mm ENIG if possible. Use BOM + positions for partial assembly. Switches and encoder better hand soldered.

### Known issues rev 3

| Issue | Detail | Workaround |
| --- | --- | --- |
| LED pins wrong | Pins swapped in schematic | Re-route or bend legs |
| Sensor noise | SI/EMI near sensor | Firmware filters most, fix would be better grounding and 3.3V routing |

Works as-is.