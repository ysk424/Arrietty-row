# Handoff

2026-09-09. User authorized an independent public MIT `ysk424/Arrietty-row`.
UE 5.8 and C++; do not change existing Arrietty-UE58 or Secret-World projects.

Accepted hardware/controls: MERACH MR-R02 (Q1S) self-powered rower; SteamVR HMD;
one VIVE Tracker 3.0 on the bar center (the former `head` assignment in sibling
manekko); the existing BLE HR monitor. Serial was extracted as local data only,
not copied with manekko source. Enter start/pause; numpad 0 stop/home. Sideways
HMD position steers; yaw does not steer. No button operations while rowing.

Required metrics: distance, time, speed, heart rate. The main panel uses boat
distance/speed, active session time, and the separate HR monitor. CSV includes
the machine metrics independently. This distinction was stated to the user.

Water quality is the main acceptance criterion. Three layers are implemented:
pixel wind waves, local mesh displacement, and dynamic pressure/ripple/foam
simulation. They must be evaluated in UE; no Blender world background is used.
Lake reference Z=0; animated displacements oscillate around it. UE sky remains
SkyAtmosphere + atmospheric sun + real-time SkyLight + height fog, as in the
Arrietty family. The accepted Bled lighting/geometry is staged read-only.

Hardware test on 2026-09-09: user explicitly rowed after a requested test.
Python diagnostics received valid Tracker movement, Q1S split FTMS notifications,
and HR notifications. Native C++ path and visual checks are separately recorded
in VALIDATION.md; do not infer full HMD acceptance from telemetry success.

Private data lives in settings.local.json and ignored logs/. Diagnostic test
scripts must not run concurrently on the same BLE peripheral. The first long
diagnostic overlapped the shorter capture when the user interrupted the turn;
the shorter capture succeeded, while the earlier client's disconnect is not a
machine failure. Diagnostics now use unique timestamped output directories.
The original 148 rower frames are additionally retained as
logs/rower-148-capture.local.json. The original HR count (120) is in the tool
output; its old fixed-name raw file was overwritten by a later zero-packet idle
check. Do not claim that a full original HR series is still on disk.

Initial C++ and editor/game targets compile; saved-map and actual-render checks
are recorded in VALIDATION.md. Preview tools create local images. Windows SDK
cppwinrt include paths are explicit; OpenVR loads from the staged binary path
or the pinned local SDK before worker threads start. No global Engine SDK copy
is needed. The native probe is tools/probe.ps1.

Known refinement areas: verify seated eye placement and lean sensitivity in
HMD. Core stroke/power model is intentionally tunable, not
a measured transfer function. Water and shoreline behavior require actual
rendering and user feedback before treating this as an accepted daily application.

Follow-up: the main keyboard Enter started a live UE session, but the rider's
keypad did not. UE maps both Enter variants to one key. Session keys now run in
a game-window-only Slate preprocessor before widget focus, suppress repeats,
and log ROW_KEY / ROW_CONTROL without device identities or health values.
The instrument widget cannot take keyboard focus. The user subsequently
confirmed repeated physical keypad operation. Do not claim
a USB hardware mapping error was established. Live UE C++ BT/HR reception and
boat movement are now verified; see updated VALIDATION.md.

That live session also exposed a shutdown crash in SteamVR/Vive OpenXR layers:
the pose worker released OpenVR before UE destroyed its OpenXR HMD. UE now
defers that release until module shutdown, after joining all device workers and
Engine PreExit/HMD teardown. The standalone probe retains immediate shutdown.

The user then reported unintended left turning and approved all four steering
changes: delayed averaged neutral, machine axis measured from bar strokes,
8 cm margin with gentle onset, and a visible left/center/right gauge. Implemented
in RowCalibration.h, RowCore.h and the UE pawn/panel. Start/resume now runs setup
before the boat moves. See STEERING.md for exact thresholds and failure behavior.
The physical center/axis stay fixed during a run. Fresh local CSV includes lean,
steering and yaw rate; never publish the user's exercise/pose samples.
