# Handoff

2026-09-10 acceptance: after accepting the dial-gain speed and requesting better
turning, the rider approved the resulting steering change and explicitly asked
to push. The combined power/display/CSV/steering changes are accepted on main.
This supersedes their earlier pending rider-evaluation status. It does not add
new quantitative hardware, dial-mapping or long-session HMD measurements.

2026-09-10 steering follow-up: the rider tried dial-gain propulsion and accepted
its speed, then reported that turning could not keep up. The fixed 0.20 rad/s
turn ceiling widened the radius as speed rose. Full-steer yaw now scales with
speed above 2 m/s for a 10 m radius, up to 0.55 rad/s at the existing 5.5 m/s boat
speed limit. Old low-speed authority/fade remain. Full lean is 20 cm instead of
26 cm; the +/-8 cm straight zone, squared onset, smoothing, calibrated axis and
no-yaw-steering rule remain. Rider acceptance is recorded above.

2026-09-10 follow-up after v1.1.0: the rider requested stronger reward for effort,
dial-level multiplication and a small watt display. Game power now uses base
watts times a fresh Q1S level 1-16. Original machine watts remain separate from
this game gain; Tracker fallback uses the same multiplier and is labeled.
The panel shows BT/Tracker-est watts, LOAD and GAME watts, including fresh machine
telemetry while stopped or paused. Unknown/invalid/stale level uses gain 1 and
shows `-- (x1)`; it must not retain an old level indefinitely. CSV appends
resistance_level, power_multiplier and game_power_w; power_w remains the original
machine value. Both watts and level expire independently after 3 s for power
calculation/display. The rider subsequently accepted the resulting speed.
The full physical dial-to-telemetry mapping still needs a hardware trial; do not
infer it from the older fixed-level capture. No BLE control writes were added.

2026-09-10 release: the rider accepted the Kelvin wake and bow-whitewater result,
said this worktree's objective was achieved, and explicitly requested push,
integration into main, adoption as the official version, and deletion of the
temporary worktree directory. Version **1.1.0** is the resulting accepted release.
The previous wait-for-integration instruction is satisfied by that explicit
approval; do not ask for it again. Future development and launches use the
original `Arrietty-row` checkout on main, not the retired worktree path.

The integrated feature history is `1551730`, `4efc03d`, `533bc91`, based on
v1.0.0 (`f45ba55`). RowKelvin.h provides dispersive gravity waves;
RowBowWhitewater.h adds speed-driven bow foam and a small visual crest. Existing
oar effects, controls, drive/coast feel and Lake Bled geometry remain.
Japanese explanation: [WATER.ja.md](WATER.ja.md). Technical model: [WATER.md](WATER.md).

Before removing the worktree, its local logs and artifacts were preserved under
`logs/kelvin-worktree/` and `artifacts/kelvin-worktree/` in the main checkout.
All Saved data is archived under `logs/kelvin-worktree/saved/`; sessions are also
copied into the normal Saved/Sessions directory without overwriting differing
records. Settings, content and SDK files were verified during local migration;
the normal checkout is rebuilt at its own path. All local evidence, settings,
sessions and generated/binary content remain excluded from Git.
Rider acceptance does not invent new quantitative HMD or long-session measures;
the recorded measurements and limits remain in [VALIDATION.md](VALIDATION.md).

2026-09-09. User authorized an independent public MIT `ysk424/Arrietty-row`.
UE 5.8 and C++; do not change existing Arrietty-UE58 or Secret-World projects.

Version 1.0.0 accepted as complete by the user on 2026-09-09, with an explicit
request to push. The user specifically praised moving forward while rowing.
Preserve that drive/coast feel as the accepted v1 baseline for future work.
The user explicitly prefers forward-facing travel for this game and considers
the backward-facing travel of competitive rowing less enjoyable here. Preserve
the forward view and forward propulsion; this is an intentional design choice.

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

Future refinement areas include seated eye placement, lean sensitivity, water
and shoreline behavior. Core stroke/power model is intentionally tunable, not
a measured transfer function. Version 1 acceptance is recorded above; further
quantitative evaluation should retain its existing measurement limits.

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

Follow-up Enter report: real input logs showed repeated calibration_begin /
calibration_cancel and later an Axis-stage timeout, not a missing Enter event.
Extra Enter during setup now preserves progress; NUM 0 is the explicit cancel.
Ready/setup instructions follow the HMD, because the physical room origin is
not yet centered. Successful calibration reattaches instruments to the boat.
Do not confuse a pending bar-motion calibration with a keyboard mapping fault.
