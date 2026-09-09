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

- Engine-independent C++: **56 checks pass** under MSVC with warnings treated as
  errors. Coverage includes FTMS split notifications and truncation, timestamp
  freshness, 8/16-bit HR and contact loss, drive/coast, eight complete strokes,
  no motion from frozen BT power, pause/reset, stale/invalid tracking, frame
  hitches, lateral steering, and no yaw steering. The moving wave domain remains
  finite and bounded through 900 fixed steps and clears on a distant teleport.
- UE **5.8.2**: Win64 Development **Editor and Game targets compile**.
- Saved rowing map reload verifies **312** scenery actors / **961,237** source
  triangles, one water actor at **Z=0**, native sky/sun/fog, and RowGameMode.
  The local animated surface adds **131,072** triangles and a **256 x 256** field.
- Actual D3D12 UE game rendering at **1600 x 1000**, with synthetic strokes:
  first-person and chase captures, advancing distance/speed, a session CSV,
  local waves, wake and foam. The latest chase capture records **27.17 m** while
  running, with no material compile failure or invalid mesh bounds. Final
  screenshots are local artifacts, not an assertion of the rider's approval.
- A native C++ console probe receives the configured Tracker's live valid poses.
  The actual UE C++ app also receives **1,133 valid bar-tracking frames** in its
  20 s device run (`-nohmd`, no motion started), loads the pinned OpenVR DLL,
  and shuts down cleanly. This tests the UE device integration, not stereo HMD
  rendering. BLE counters remain zero during that idle run.
  The later rower scan found no advertisement; HR advertised but also sent zero
  notifications in a separate Python check after the exercise had ended.

## Remaining physical acceptance

The full C++ path needs another awake-rower / worn-HR notification test. The
successful 148/120 notification exercise above used Python diagnostics. Do not
report the C++ BLE path as live-accepted until its counters advance.

Actual HMD operation, keypad reach, seated eye position, lean calibration,
comfort, stereo frame time and the user's judgment of wave appearance remain
open. Desktop screenshots and offline model checks do not establish these.

Local evidence: logs/ue-build.log, logs/ue-game-build.log,
logs/content-validation.json, logs/row-wake.log, artifacts/row-view.png,
artifacts/row-wake.png and Saved/Sessions CSV files. Full notification captures
from future diagnostics use unique timestamped directories to avoid overwrites.
