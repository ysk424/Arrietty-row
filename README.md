# Arrietty-row

UE 5.8 / C++ rowing on Lake Bled, Slovenia. Independent of Arrietty-UE58.
MERACH MR-R02 (Q1S), a SteamVR HMD, one VIVE Tracker 3.0 at the handle
center, and a Bluetooth heart-rate monitor.

日本語の操作手順: [USAGE.ja.md](docs/USAGE.ja.md)

The application code is MIT licensed. Unreal Engine, the separately prepared
Lake Bled scenery, and downloaded OpenVR SDK retain their own licenses.
This is an initial implementation; live HMD comfort and water appearance need
the rider's acceptance. See [validation](docs/VALIDATION.md) for measured results.

## Use

1. Install UE 5.8 and Visual Studio C++ tools with a Windows SDK.
2. Prepare the separate Lake Bled UE source project, with map
   `/Game/Worlds/LakeBled/Maps/LakeBled`. See [scenery](docs/SCENERY.md).
3. Run `./tools/prepare.ps1`. This downloads a pinned OpenVR SDK, compiles C++,
   copies local scenery, and creates materials and the rowing map.
4. Copy `settings.example.json` to `settings.local.json`. Set the exact handle
   Tracker serial and the rower/heart monitor BLE addresses. Local settings,
   diagnostic data, and sessions are ignored by Git. Never commit device IDs.
5. Set SteamVR as the active OpenXR runtime, start SteamVR and the devices,
   then run `./tools/run.ps1`. Use `./tools/run.ps1 -Demo` without hardware.

Sit normally, look along the rowing machine, extend the handle, and press
**Enter** on the keypad to start. Enter also pauses and resumes. **Numpad 0**
stops, saves the session, and returns the boat to its starting position.
Insert (Num Lock off) and Esc also stop. Lean sideways to turn; head rotation
alone does not steer. After tracking loss, restore tracking and press Enter.

The panel shows **distance, active time, speed, and heart rate**. Distance and
speed describe the virtual boat's movement. The CSV separately records machine
distance, elapsed time, speed (from 500 m pace), and watts. Paused time is
excluded from active time; missing/stale heart rate is `--`, never 0 bpm.
Sessions stay under `unreal/ArriettyRow/Saved/Sessions/`.

## Rowing and water

Windows C++/WinRT receives FTMS Rower Data and Heart Rate notifications.
OpenVR reads the exact configured Tracker by serial, regardless of its old
SteamVR role. OpenXR renders the HMD. BLE power takes priority; handle velocity
times thrust to the drive, and the boat coasts during recovery. When BLE power
is missing, the panel labels a Tracker-based estimate. It is a rowing feel model,
not a calibrated ergometer. Bluetooth never changes the machine's resistance.

The mean water plane is **Z=0 cm**. Wind ripples use five world-space normal
waves; a dense local surface adds small geometric lake waves. A moving 64 m
height field propagates bow/stern and oar impulses, leaving curved wakes and
decaying foam in world space. The HMD horizon stays level while the hull moves.
The approved Lake Bled mesh mountains and UE atmosphere form the distant view.

## Development

`./tools/build_native.ps1` runs the engine-independent C++ checks.
`./tools/build_native.ps1 -Devices` builds the native hardware probe.
`tools/diagnose.py` is an optional Python diagnostic tool; Python is not part
of the simulator runtime. The editor content generator also uses UE's Python.

Read [handoff](docs/HANDOFF.md), [validation](docs/VALIDATION.md), and
[third-party notices](THIRD_PARTY_NOTICES.md).
