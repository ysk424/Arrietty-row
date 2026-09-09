# Validation

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

## Remaining physical acceptance

The later real UE/OpenXR session received **1,316 rower packets**, **1,295 HR
packets**, and **59,497 valid Tracker frames**, with zero rejected rower packets.
Its private CSV contains fresh BT power and HR, advancing virtual distance, and
start/exit records. This verifies live C++ reception separately from the earlier
Python exercise. The user subsequently confirmed multiple runs using the
physical keypad and reported left drift. The new calibration/margin/gauge
changes address that feedback; their physical steering feel still needs review.

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

The new setup flow, seated eye position, lean calibration, comfort, stereo frame
time and the user's judgment of wave appearance remain open for physical
acceptance. Desktop screenshots and offline model checks do not establish these.

Local evidence: logs/ue-build.log, logs/ue-game-build.log,
logs/content-validation.json, logs/row-wake.log, artifacts/row-view.png,
artifacts/row-wake.png and Saved/Sessions CSV files. Full notification captures
from future diagnostics use unique timestamped directories to avoid overwrites.
