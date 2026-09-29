# Configuration Guide

Mouse configured in browser via WebHID. No app.

Site: https://realsynarix.github.io/Cobalt-X-Zero

### Access

| Step | Action |
| --- | --- |
| 1 | Chrome or Edge browsers (Desktop) |
| 2 | Open site, plug mouse |
| 3 | Connect, pick Cobalt-X Zero |
| 4 | Create Profile, Re-Read All, edit, Save Changed |

### FIDs

Stored at 0x0801F800 with CRC32.

| FID | Name | Type | Purpose |
| --- | --- | --- | --- |
| 1 | device_name | string 31 | OS name |
| 2 | manufacturer | string 31 | Default Synarix |
| 3 | serial | string 15 | Serial |
| 4 | fw_version | string 15 | 0.9.2 read-only |
| 16-21 | mod flags | bool | Enable modules |
| 32 | dpi | u16 | 100-32000 |
| 33 | poll | u16 | 125,250,500,1000,2000,4000,8000 |
| 34 | lod_mm | u8 | 1-3 |
| 35 | lod_squal | u8 | 10-200 |
| 36 | angle_snap | bool | Snap |
| 37 | angle_strength | u8 | 0-100 |
| 38 | surface | u8 | 0-3 |
| 48-52 | debounce | u8 | 0-20ms per PB |
| 53 | click_mode | u8 | 0-2 |
| 64 | wheel_div | u8 | 1-8 default 3 |
| 65 | wheel_inv | bool | Invert |
| 66 | wheel_smooth | bool | Smooth |
| 80-85 | hyprx | mixed | HyprX engine |
| 96 | led_def | rgb | Default #0A5BD7 |
| 97 | led_macro | rgb | Macro |
| 98 | led_bri | u8 | 0-255 brightness |
| 99 | led_eff | u8 | 0 solid 1 breathing 2 blink 3 rainbow 4 chase |
| 100 | led_speed | u8 | Speed |
| 112-117 | pb7 | mixed | PIO button modes |
