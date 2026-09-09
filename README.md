# Arrietty-row

UE 5.8 / C++ rowing on Lake Bled, Slovenia. Independent of Arrietty-UE58.
MERACH MR-R02 (Q1S), a SteamVR HMD, one VIVE Tracker 3.0 at the handle
center, and a Bluetooth heart-rate monitor.

日本語の操作手順: [USAGE.ja.md](docs/USAGE.ja.md)

ケルビン波の実装・実機確認・統合の手順: [WATER.ja.md](docs/WATER.ja.md)

The application code is MIT licensed. Unreal Engine, the separately prepared
Lake Bled scenery, and downloaded OpenVR SDK retain their own licenses.
Version **1.0.0** was accepted by the rider on **2026-09-09**, with positive
feedback on the feeling of moving forward while rowing.
Forward-facing travel is intentional: the rider enjoys the lake and water
ahead while each drive propels the boat forward.
See [validation](docs/VALIDATION.md) for measured results and evaluation scope.

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

Press **Enter** on the keypad, return to your normal posture during the two-second
countdown, hold still for one second, then move the handle straight out and back
twice. The boat starts automatically after measuring the machine axis. Setup
does not count as exercise or move the boat. Enter also pauses and resumes
(with fresh calibration). Extra Enter presses during setup preserve progress.
**Numpad 0** cancels setup or
stops, saves the session, and returns the boat to its starting position.
Insert (Num Lock off) and Esc also stop. Lean sideways to turn; head rotation
alone does not steer. After tracking loss, restore tracking and press Enter.
Both keypad and main Enter work in the active game window. Session keys are
handled before widget focus; holding a key causes only one action.
The averaged neutral has an **8 cm straight zone on each side** and a gentle
turn response outside it. A live LEFT / CENTER / RIGHT gauge shows the same
filtered position used for steering. See [steering](docs/STEERING.md).

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
height field combines dispersive bow/stern waves with oar ripples, leaving curved
wakes and decaying foam in world space. The HMD horizon stays level while the hull moves.
The approved Lake Bled mesh mountains and UE atmosphere form the distant view.

The `feat/kelvin-wake` worktree adds a deep-water dispersive bow/stern solver:
transverse waves, divergent waves and curved wake history. Oar ripples and foam
remain separate. See [Kelvin wake development](docs/WATER.md) for the model,
offline previews and checks. This feature is not yet merged into the accepted
training version on `main`.

## Development

`./tools/build_native.ps1` runs the engine-independent C++ checks.
`./tools/build_native.ps1 -Devices` builds the native hardware probe.
`python tools/test_keypad.py` checks keypad/main Enter, repeat suppression, and
stop keys through Windows messages in a separate offline UE game window.
`./tools/test_setup.ps1` runs a UE automation regression for repeated Enter
during setup, stop/retry, and camera/boat attachment of the instruction panel.
`tools/diagnose.py` is an optional Python diagnostic tool; Python is not part
of the simulator runtime. The editor content generator also uses UE's Python.

Read [handoff](docs/HANDOFF.md), [validation](docs/VALIDATION.md), and
[third-party notices](THIRD_PARTY_NOTICES.md).
