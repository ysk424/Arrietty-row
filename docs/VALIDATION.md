# Validation

## Bow whitewater follow-up, 2026-09-10

The rider requested whitewater at the bow that responds to boat speed. This
change is in the same `feat/kelvin-wake` worktree and is not merged into main.

- MSVC `/W4 /WX /O2`: **76 core checks and 46 water checks pass**. Added checks
  cover zero/invalid/very low speed, increasing foam at 1/2/3 m/s, emission ahead
  of the actual 2.3 m hull tip, left/right symmetry, heading rotation, elapsed-time
  emission consistency, world-space persistence, stopping/decay, bounded crest,
  unchanged oar foam/height fields, and clearing on teleport. Kelvin dispersion
  and propagation checks also remain successful. Evidence:
  `logs/native-bow-water.log`.
- Final UE **5.8.2 Editor and Game Development targets compile**, with no C++
  warnings in `logs/ue-bow-build.log` and `logs/ue-bow-game-build.log`.
- The production UE setup/control test passes on rerun without a runtime code
  change: `logs/setup-controls-20260910-080048.log`. The first attempt ended
  after the automation controller dispatched the test, with no completion or
  control-handler records (`setup-controls-20260910-075843.log`). Its cause was
  not established; do not count that first attempt as a pass. The test launcher
  now includes the process exit code in future failure messages.
- Actual **1600 x 1000 D3D12** synthetic straight-rowing captures show whitewater
  outside the bow and along its shoulders. The dedicated bow foam channel fades
  faster than the existing oar foam, keeping the new whitewater concentrated near
  the bow. First-person and chase captures both record a running session at
  **10.15 m** and exit normally. The log identifies `bow_whitewater=speed`.
  Evidence: `artifacts/row-bow-view.png`, `artifacts/row-bow-chase.png` and the
  corresponding `logs/row-bow-*.log`.
- The same copied water material displays the combined foam and crest field;
  generated material/scenery assets were not rebuilt for this change. The
  additional crest contributes to both height and its normal derivatives.

These checks use synthetic inputs. Bow whitewater is an artistic effect, with
no particle/breaking-fluid simulation. Hardware/HMD appearance, sustained frame
time and training comfort still require testing in the worktree. They do not
constitute rider acceptance or permission to integrate into main.

## Kelvin feature worktree, 2026-09-10

This is development evidence for `feat/kelvin-wake`, not approval to replace the
accepted training version. The rider reported another successful training run
on that version and requested isolated worktree development until acceptance.
The original checkout remains at `f45ba55`; no merge or push was performed.

- MSVC `/W4 /WX /O2`: **76 existing core checks and 28 water checks pass**.
  The water checks cover two independent Fourier-mode dispersion oracles, no
  new wake at rest, invalid speed, steady motion at 2 and 3 m/s, bilateral
  symmetry, bounded amplitude, reference-level drift, exact world-space sample
  translation, stopping/decay, teleport clearing both state fields, turns, and
  export of synthetic evidence. Evidence: `logs/native-water.log`.
- Measured transverse wavelengths in exported C++ fields: **2.50 m** at 2 m/s
  (theory 2.562 m), **5.875 m** at 3 m/s (theory 5.764 m). `plot_water.py` also
  checks mean-square displacement outside the bow's theoretical wedge: the
  26–55 degree sector has **0.077% / 0.083%** of the within-21-degree sector's
  mean-square displacement, sampled 9–23 m astern. This checks confinement;
  it is not an exact measurement of the cusp angle. Evidence:
  `artifacts/water/measurements.json` and `kelvin-validation.png` in that folder.
- After 40 s of straight motion, interior peak displacements are **4.32 cm /
  6.94 cm** for those speeds, with mirror differences below 0.003 mm. Thirty
  seconds after removing pressure, integrated squared height is **2.65% /
  1.67%** of its moving value. This is a displacement measure, not full physical
  wave energy. The 40 s turning fixture retains a curved wake history.
- The 5,400 timed fixed steps average **1.54 ms per 30 Hz update** on this PC,
  including domain recentering and test bookkeeping. This is a standalone CPU
  measurement, not UE frame time, GPU cost, a worst-case bound or VR performance.
- UE **5.8.2 Editor and Game Development targets compile**, without C++ warnings
  in the final build logs: `logs/ue-build.log`, `logs/ue-game-build.log`.
- `tools/test_setup.ps1` passes the production UE control regression for extra
  Enter, setup completion, pause, stop/retry and panel attachment. Evidence:
  `logs/setup-controls-20260910-065743.log`. No new physical keypad/USB test or
  BLE/Tracker exercise was performed for this feature.
- Saved-map verification still reports **312 scenery mesh actors / 961,237
  source triangles**, native atmosphere/sun/sky/fog, and water reference
  **Z=0 cm**. Evidence: `logs/content-validation.json` and
  `logs/ue-verify-content.log`. The worktree uses its own local content copy.
- Actual D3D12 UE **1600 x 1000** synthetic previews render the combined field,
  hull, oars and foam during straight motion and turns. The elevated aft view
  captures after 52 seconds with a running session; both runs close normally.
  The water log identifies `kelvin=deep_water_fft hz=30`. Evidence:
  `artifacts/row-kelvin-straight.png`, `artifacts/row-kelvin-turn.png` and their
  corresponding `logs/*.log`. Wake contrast depends strongly on view and sun
  reflection; the field plot isolates the physical wave pattern from wind and
  shading. These captures do not measure the rendered envelope angle or imply
  rider acceptance of its appearance.
- Normal first-person and chase previews also close normally after capturing a
  running synthetic session at **10.15 m**, with the four metrics, missing HR
  shown as `--`, the +/-8 cm steering gauge and the existing distant scenery.
  Evidence: `artifacts/row-view.png`, `artifacts/row-wake.png` and corresponding
  logs. The copied map and water-material asset hashes match the training
  checkout; no generated scenery or material was changed.

No stereo HMD comfort, long training session or sustained UE frame-time test
has been run on this feature. The finite local deep-water model's physical and
rendering limits are described in [WATER.md](WATER.md).

## Accepted version 1.0.0

Version **1.0.0** was accepted as complete by the user on **2026-09-09**.
The user specifically praised the feeling of moving forward while rowing.
This records rider acceptance alongside the measured checks below.

## Measured hardware data, 2026-09-09

- Configured bar Tracker connected; 5,262 valid samples in the 55 s capture.
  Largest room-axis displacement was about 0.97 m during the user's rowing.
- Q1S FTMS Rower Data: 148 notifications, 74 pairs. Flags `0x00ff` carry
  average cadence, distance, pace, power and resistance; flags `0x0b00` carry
  cadence/count, energy, heart-rate placeholder and elapsed time.
- Separate HR monitor: 120 notifications. Do not use the machine's zero HR
  placeholder instead of the chest/arm monitor.
- Distance advanced to 114 m and stroke count to 19; stopped samples had zero
  instantaneous pace, power and cadence. Device identities and captured personal
  values are excluded from this public document.
- These measurements used the optional Python diagnostic, not UE rendering.

## Verified implementation

- Engine-independent C++: **76 checks pass** under MSVC with warnings treated as
  errors. Coverage includes FTMS split notifications and truncation, timestamp
  freshness, 8/16-bit HR and contact loss, drive/coast, eight complete strokes,
  no motion from frozen BT power, pause/reset, stale/invalid tracking, frame
  hitches, lateral steering, and no yaw steering. The moving wave domain remains
  finite and bounded through 900 fixed steps and clears on a distant teleport.
  Steering regressions cover both 8 cm margins, symmetric turns, gentle onset,
  no turn after recentering, delayed averaged neutral that excludes keypad reach,
  two-stroke machine-axis fitting, and 40 cm fore/aft head travel with a 20-degree
  gaze offset. Invalid calibration, moving posture, stationary timeout, tracking
  loss, frame hitches and nonlinear bar motion cannot silently start a run.
- UE **5.8.2**: Win64 Development **Editor and Game targets compile**.
- Saved rowing map reload verifies **312** scenery actors / **961,237** source
  triangles, one water actor at **Z=0**, native sky/sun/fog, and RowGameMode.
  The local animated surface adds **131,072** triangles and a **256 x 256** field.
- Actual D3D12 UE game rendering at **1600 x 1000**, with synthetic strokes:
  first-person and chase captures, advancing distance/speed, a session CSV,
  local waves, wake and foam. The latest chase capture records **27.17 m** while
  running, with no material compile failure or invalid mesh bounds. Final
  screenshots are local artifacts, not an assertion of the rider's approval.
  The updated first-person panel is visually checked in a real UE render:
  four metrics, a green +/-8 cm band, live marker, RIGHT indication and signed
  displacement all fit. Its synthetic session starts at zero exercise time and
  distance after calibration, reaches 10.15 m at capture, and writes the three
  steering columns. Evidence: logs/row-view.log and artifacts/row-view.png.
- A native C++ console probe receives the configured Tracker's live valid poses.
  The actual UE C++ app also receives **1,133 valid bar-tracking frames** in its
  20 s device run (`-nohmd`, no motion started), loads the pinned OpenVR DLL,
  and shuts down cleanly. This tests the UE device integration, not stereo HMD
  rendering. BLE counters remain zero during that idle run.
  The later rower scan found no advertisement; HR advertised but also sent zero
  notifications in a separate Python check after the exercise had ended.

## Live operation and version 1 acceptance

The later real UE/OpenXR session received **1,316 rower packets**, **1,295 HR
packets**, and **59,497 valid Tracker frames**, with zero rejected rower packets.
Its private CSV contains fresh BT power and HR, advancing virtual distance, and
start/exit records. This verifies live C++ reception separately from the earlier
Python exercise. The user subsequently confirmed multiple runs using the
physical keypad and reported left drift. The new calibration/margin/gauge
changes address that feedback and are included in the accepted version 1.

The earlier `tools/test_keypad.py` run passed **11 Windows key presses** in a separate
offline UE game: extended keypad Enter start/pause/resume/restart, main Enter,
numpad 0 with Num Lock on, Insert with Num Lock off, Esc, and setup cancellation.
Each press includes three repeat messages; each creates one input command.
Start/resume commands run the real setup state machine with synthetic physical
poses, then automatically start after a completed measured calibration. All
commands pass through the Slate preprocessor and the offline game exits with
code 0. This automation does not test USB reports or input to an inactive app.

After the next real Enter report, input logs showed setup begin/cancel cycles
and an axis-stage timeout. Enter delivery itself was working. Setup now ignores
additional Enter presses while preserving progress; NUM 0 cancels explicitly.
`tools/test_setup.ps1` passes the UE automation test
`ArriettyRow.Controls.SetupEnter`: five extra Enter actions preserve a partially
completed setup, calibration completes, running Enter pauses, NUM 0 clears
setup, and the panel follows the camera until returning to the boat on start.
This test invokes the production control handlers in an offline UE game and
does not simulate Windows key delivery. Evidence: logs/setup-controls-20260909-175953.log.
An offscreen UE capture with the camera displaced from the boat confirms the
setup panel stays in view, showing `2/3 HOLD STILL` and zero exercise metrics.
Evidence: artifacts/row-setup.png (`tools/preview.ps1 -Setup -Chase`). The capture
itself causes a frame hitch and cancels setup afterwards; completion behavior
is verified separately by the UE automation test above.

The original live session crashed during HMD teardown after its CSV exit record
was saved. Deferring OpenVR release until after OpenXR teardown removes that
failure in a subsequent 15-second real SteamVR/OpenXR run: device workers stop,
the engine shuts down, ROW_OPENVR_SHUTDOWN follows, and the log closes normally
without a critical/fatal error. Evidence: logs/vr-shutdown-20260909-170504.log.

Version 1 acceptance does not add quantitative stereo frame-time or long-session
comfort measurements. Those remain future evaluation topics; the automated
checks and screenshots above retain their stated scope.

Local evidence: logs/ue-build.log, logs/ue-game-build.log,
logs/content-validation.json, logs/row-wake.log, artifacts/row-view.png,
artifacts/row-wake.png and Saved/Sessions CSV files. Full notification captures
from future diagnostics use unique timestamped directories to avoid overwrites.
