# Fabrication Files

All the files needed to edit and manufacture the PCB are in this folder.

Known issues (as of the current revision):

1. LED pin locations are wrong in the schematic.
   Just move the pins in the schematic and re-route the traces.
   If you don’t, you’ll have to solder the LEDs with bent pins (it’s annoying).

2. The sensor area has noticeable SI / noise problems (probably from the power rails or general EMI).
   The current firmware cleans most of it up, but it would be much better to fix the layout and then re-tune the firmware before you order boards.

These issues don’t make the board unusable. You can fabricate as-is and it will still work.
If you want the mouse to be as good as possible, I strongly recommend fixing both points first.

(you can then submit a PR with the fixes if you like :D)

I never fixed them myself, the project already cost me over $200 and I ran out of time/energy.

Regarding the BOM, you will notice many parts not included. That is due to the fact that JLCPCB either does not have them, or its best to get them yourself and solder.
