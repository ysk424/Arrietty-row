# Validation

## Firefly rowing audio and left-ear balance, 2026-09-11

- All five final user-supplied masters import as 48 kHz stereo PCM SoundWaves.
  Original file hashes remain unchanged. Prepared L/R energy differences are
  within 0.00015 dB of +6 dB; peaks are at most 0.5. Loop boundary differences
  are below the respective 99th-percentile natural adjacent-sample changes.
  Evidence: `logs/audio-content-20260911-064251.log`,
  `artifacts/audio/preparation.json`, `artifacts/audio/seams.json`.
- MSVC `/W4 /WX /O2`: **104 core + 46 water + 16 audio checks pass**.
  Audio checks cover state gating, one catch per registered stroke, recovery,
  coasting, stopping, resetting, invalid values, mix headroom and no metric
  advancement. Evidence: `logs/native-audio.log`.
- UE **5.8.2 Editor and Game Development targets build successfully**. The
  final incremental builds contain no C++ warnings. The first full rebuild
  reports existing float-literal conversion warnings in unchanged RowPanel.cpp.
  Evidence: `logs/ue-audio-editor-build-final.log`,
  `logs/ue-audio-game-build-final.log` and the corresponding initial build logs.
- The production UE asset automation passes: all five voices have the expected
  mono-folded stereo sources, PCM codec, inline loading and looping flags;
  spatialization and multiple instances are disabled. The offline demo emits
  six catch triggers during capture. The final complete `test_audio.ps1` run
  passes. Its **30.016 s** actual mixer recording contains audio in all 30 full
  one-second windows and the final fragment, measures **left +6.0079 dB**,
  peaks at **-17.06 dBFS**, and has no clipped samples. Every left/right sample
  pair matches the requested gain within **1.266 PCM units** (16-bit export).
  The longest consecutive digital silence is **0.146 ms**.
  Evidence: `logs/audio-runtime-20260911-065456.log`,
  `artifacts/audio/ue-mix-20260911-065456.wav` and matching `.json`.
- Existing production setup/Enter/pause/stop/panel-attachment automation passes:
  `logs/setup-controls-20260911-065232.log`. This does not retest physical USB
  key delivery. No controls, propulsion, calibration, steering, camera or Lake
  Bled scenery algorithms/assets were changed by this audio addition.

Capture-harness corrections are retained honestly: the first 36-second
launcher exited before its simulation-time recording timer completed after
startup stalls (`audio-runtime-20260911-064624.log`). Capture now starts after
warmup, uses wall-clock duration, and has a longer exit allowance. The second
capture was silent because the offscreen window used UE's default unfocused
volume of zero (`audio-runtime-20260911-064849.log`); the test now overrides
that setting only for its own process. The first non-silent capture
(`artifacts/audio/ue-mix-20260911-065036.wav`, left +6.0078 dB, -17.40 dBFS)
exceeded an overly tight per-window 0.05 dB RMS tolerance at quiet levels due
to UE's integer WAV export. The verifier now checks every sample against the
6 dB ratio with a 2.5-unit source/export quantization bound, plus whole-recording
balance within 0.05 dB; this same recording passes those checks. A later uncapped
replay failed synthetic calibration and emitted no catches
(`audio-runtime-20260911-065317.log`); it is not counted as a pass. The final
launcher caps its own frame rate to 60 for steadier synthetic timing. No live
tracking watchdog or calibration rule was relaxed.

The rider subsequently confirmed that the sound was OK on 2026-09-11 and
explicitly requested push. This records acceptance of the sound result.
The quantitative checks above remain synthetic game-output measurements, not
an instrumented headphone listening test, OS/headphone channel calibration,
live Tracker/BLE trial or long-session comfort measurement. Sound sources are centered in the listener's
ears; this implementation does not provide 3D sound localization. See
[audio guide](AUDIO.ja.md). Generated assets, recordings and sessions remain local.

## Steering at the accepted higher speed, 2026-09-10

The rider subsequently approved dial-gain speed and reported insufficient
turning authority. The new steering model reaches full input at 20 cm and
maintains a 10 m full-steer target radius above 2 m/s. See `STEERING.md` for the
curve, low-speed behavior and smoothing. The +/-8 cm straight zone is unchanged.

- **104 core + 46 water checks pass**: `logs/native-steering-speed.log`.
  New steering checks hold full-steer radius within 0.002 m of 10 m at fixed
  speeds 2, 3.7 and 5.5 m/s; verify the 18/20 cm response, gentle 10 cm onset,
  preserved low-speed authority, zero stationary target and bounded turn rate.
  Existing symmetric steering, immediate CENTER response, calibration and
  no-head-yaw-steering checks still pass.
- UE **5.8.2 Editor and Game Development targets build successfully**:
  `logs/ue-steering-speed-editor-build.log` and
  `logs/ue-steering-speed-game-build.log`.
- UE setup/repeated Enter/pause/stop/panel-attachment automation passes:
  `logs/setup-controls-20260910-205114.log`.
- An actual offscreen D3D12 demo, without a screenshot during the run, records
  **19.417 active seconds / 43.294 m** and exits normally with no tracking-loss
  events. Its 22 CSV rows match the new lean curve, retain original/game watts
  at synthetic 95/570 W, and show turn rates above the old speed-independent
  limit for the same steering input. Peak speed is 12.532 km/h and peak turn
  rate is 13.225 degrees/s in this varying-lean demo. Evidence:
  `logs/row-steering-speed-demo.log` and its ignored synthetic session CSV.
- The rider subsequently approved the steering change and explicitly requested
  push, accepting the combined power and steering behavior. The runtime figures
  above are from a short synthetic check; this approval does not add quantitative
  physical turning-radius or long-session HMD measurements.

## Dial gain and watt display follow-up, 2026-09-10

The rider requested dial-level multiplication to reward effort in the game.
This follows the accepted v1.1.0. The rider later accepted the speed and subsequent
steering improvements and requested push, as recorded above.

- `tools/build_native.ps1`: **95 core + 46 water checks pass** under MSVC.
  New coverage includes resistance parsing and independent expiry, live gain
  increases/decreases, zero watts, invalid/missing/stale levels, Tracker fallback,
  increasing distance with identical synthetic strokes at levels 1/6/16, and
  no propulsion from a stationary handle even with maximum level and BT watts.
- UE **5.8.2 Editor and Game Development builds succeed**. Evidence:
  `logs/ue-load-build.log`, `logs/ue-load-game-build.log`.
- Existing UE setup, repeated Enter, pause, stop/retry and instrument attachment
  automation passes: `logs/setup-controls-20260910-203653.log`.
- Actual **1600 x 1000 D3D12** renders were inspected with the final text layout:
  `artifacts/row-load-watts-final.png` shows `BT 95 W | LOAD 6 | GAME 570 W`;
  `artifacts/row-load-unavailable.png` shows unavailable watts as `--` and
  `LOAD -- (x1)`. Both retain the four main metrics and the +/-8 cm gauge without
  clipping. Matching logs are under `logs/row-load-*.log`.
- The final synthetic session has **16 rows and 18 columns**. CSV verification
  confirms original 95 W, level/multiplier 6 and game 570 W remain separate,
  source is `demo`, initial distance/time are zero, distance advances and missing
  HR remains blank. It ends at 12.075 m. The capture session enters
  `tracking_lost` after the screenshot and then exits normally; this short
  render/CSV check does not establish continuous-run or HMD timing stability.
- These automated checks used synthetic strokes. No instrumented physical
  dial-change trial was performed. The old fixed-level capture cannot establish
  the full 1-16 dial mapping or whether the machine's own watt calculation already
  accounts for resistance.
  GAME watts are an intentional game gain, not measured human watts. The rider
  subsequently accepted the stronger drive response; the full dial mapping
  remains outside this test's scope.

## Accepted version 1.1.0 and integration, 2026-09-10

The rider stated that the worktree objective was achieved and the result was
in very good shape, and explicitly requested push, adoption as the official
version on main, continued development in the original checkout, and removal
of the temporary worktree. This is rider acceptance of **v1.1.0**, including
Kelvin wakes and speed-driven bow whitewater. The integration preserves the
feature commits `1551730`, `4efc03d` and `533bc91` above the v1.0.0 baseline.

Verification repeated in the original `Arrietty-row` checkout:

- **76 core + 46 water checks pass**, along with the two-speed wavelength and
  confinement checks. Evidence: `logs/native-v1.1.0.log`,
  `logs/water-plot-v1.1.0.log`, `artifacts/water/measurements.json`.
- UE **5.8.2 Editor and Game Development targets rebuild successfully at the
  normal checkout path**. Evidence: `logs/ue-v1.1.0-editor-build.log` and
  `logs/ue-v1.1.0-game-build.log`.
- A real D3D12 **1600 x 1000** offline demo renders the wake and bow whitewater,
  records **10.15 m** while running, and exits normally. The log identifies
  both `kelvin=deep_water_fft` and `bow_whitewater=speed`. Evidence:
  `artifacts/row-v1-1-0-view.png`, `logs/row-v1-1-0-view.log`.
- The production UE calibration/Enter/pause/stop/panel-attachment regression
  passes: `logs/setup-controls-20260910-081923.log`.
- Saved-map verification retains **312 mesh actors / 961,237 source triangles**,
  native atmosphere and **water Z=0 cm**: `logs/ue-v1.1.0-content.log` and
  `logs/content-validation.json`.

Before cleanup, 565 original local files were verified against their retained
copies. Worktree logs/artifacts are archived under `logs/kelvin-worktree/` and
`artifacts/kelvin-worktree/`; all Saved data is retained under
`logs/kelvin-worktree/saved/`. Fifteen session files were also transferred into
the normal session directory without overwriting differing records. Settings,
content and the pinned SDK were preserved/verified. No personal data, generated
assets or binaries are added to Git. Regenerable worktree build/cache files can
be discarded. Local migration manifests remain under ignored `logs/`.

The following sections preserve the earlier development evidence and its
limits. Their pending-acceptance state has been superseded by the explicit
v1.1.0 acceptance above. No new quantitative stereo frame-time or long-session
comfort measurements are inferred from that acceptance.

## Bow whitewater follow-up, 2026-09-10

The rider requested whitewater at the bow that responds to boat speed. These
checks were performed in `feat/kelvin-wake` before its acceptance as v1.1.0.

- MSVC `/W4 /WX /O2`: **76 core checks and 46 water checks pass**. Added checks
  cover zero/invalid/very low speed, increasing foam at 1/2/3 m/s, emission ahead
  of the actual 2.3 m hull tip, left/right symmetry, heading rotation, elapsed-time
  emission consistency, world-space persistence, stopping/decay, bounded crest,
  unchanged oar foam/height fields, and clearing on teleport. Kelvin dispersion
  and propagation checks also remain successful. Evidence:
  `logs/kelvin-worktree/native-bow-water.log`.
- Final UE **5.8.2 Editor and Game Development targets compile**, with no C++
  warnings in `logs/kelvin-worktree/ue-bow-build.log` and `logs/kelvin-worktree/ue-bow-game-build.log`.
- The production UE setup/control test passes on rerun without a runtime code
  change: `logs/kelvin-worktree/setup-controls-20260910-080048.log`. The first attempt ended
  after the automation controller dispatched the test, with no completion or
  control-handler records (`setup-controls-20260910-075843.log`). Its cause was
  not established; do not count that first attempt as a pass. The test launcher
  now includes the process exit code in future failure messages.
- Actual **1600 x 1000 D3D12** synthetic straight-rowing captures show whitewater
  outside the bow and along its shoulders. The dedicated bow foam channel fades
  faster than the existing oar foam, keeping the new whitewater concentrated near
  the bow. First-person and chase captures both record a running session at
  **10.15 m** and exit normally. The log identifies `bow_whitewater=speed`.
  Evidence: `artifacts/kelvin-worktree/row-bow-view.png`, `artifacts/kelvin-worktree/row-bow-chase.png` and the
  corresponding `logs/kelvin-worktree/row-bow-*.log`.
- The same copied water material displays the combined foam and crest field;
  generated material/scenery assets were not rebuilt for this change. The
  additional crest contributes to both height and its normal derivatives.

These checks use synthetic inputs. Bow whitewater is an artistic effect, with
no particle/breaking-fluid simulation. These measurements do not quantify
HMD frame time or long-session comfort. Subsequent rider acceptance and
integration authorization are recorded separately above.

## Kelvin feature worktree, 2026-09-10

This is historical development evidence for `feat/kelvin-wake`. The rider had
reported another successful run on v1.0.0 and requested isolated development.
At the time of these checks the original checkout remained at `f45ba55`, with
integration deferred. Subsequent v1.1.0 acceptance supersedes that pending state.

- MSVC `/W4 /WX /O2`: **76 existing core checks and 28 water checks pass**.
  The water checks cover two independent Fourier-mode dispersion oracles, no
  new wake at rest, invalid speed, steady motion at 2 and 3 m/s, bilateral
  symmetry, bounded amplitude, reference-level drift, exact world-space sample
  translation, stopping/decay, teleport clearing both state fields, turns, and
  export of synthetic evidence. Evidence: `logs/kelvin-worktree/native-water.log`.
- Measured transverse wavelengths in exported C++ fields: **2.50 m** at 2 m/s
  (theory 2.562 m), **5.875 m** at 3 m/s (theory 5.764 m). `plot_water.py` also
  checks mean-square displacement outside the bow's theoretical wedge: the
  26–55 degree sector has **0.077% / 0.083%** of the within-21-degree sector's
  mean-square displacement, sampled 9–23 m astern. This checks confinement;
  it is not an exact measurement of the cusp angle. Evidence:
  `artifacts/kelvin-worktree/water/measurements.json` and `kelvin-validation.png` in that folder.
- After 40 s of straight motion, interior peak displacements are **4.32 cm /
  6.94 cm** for those speeds, with mirror differences below 0.003 mm. Thirty
  seconds after removing pressure, integrated squared height is **2.65% /
  1.67%** of its moving value. This is a displacement measure, not full physical
  wave energy. The 40 s turning fixture retains a curved wake history.
- The 5,400 timed fixed steps average **1.54 ms per 30 Hz update** on this PC,
  including domain recentering and test bookkeeping. This is a standalone CPU
  measurement, not UE frame time, GPU cost, a worst-case bound or VR performance.
- UE **5.8.2 Editor and Game Development targets compile**, without C++ warnings
  in the final build logs: `logs/kelvin-worktree/ue-build.log`, `logs/kelvin-worktree/ue-game-build.log`.
- `tools/test_setup.ps1` passes the production UE control regression for extra
  Enter, setup completion, pause, stop/retry and panel attachment. Evidence:
  `logs/kelvin-worktree/setup-controls-20260910-065743.log`. No new physical keypad/USB test or
  BLE/Tracker exercise was performed for this feature.
- Saved-map verification still reports **312 scenery mesh actors / 961,237
  source triangles**, native atmosphere/sun/sky/fog, and water reference
  **Z=0 cm**. Evidence: `logs/kelvin-worktree/content-validation.json` and
  `logs/kelvin-worktree/ue-verify-content.log`. The worktree uses its own local content copy.
- Actual D3D12 UE **1600 x 1000** synthetic previews render the combined field,
  hull, oars and foam during straight motion and turns. The elevated aft view
  captures after 52 seconds with a running session; both runs close normally.
  The water log identifies `kelvin=deep_water_fft hz=30`. Evidence:
  `artifacts/kelvin-worktree/row-kelvin-straight.png`, `artifacts/kelvin-worktree/row-kelvin-turn.png` and their
  corresponding `logs/kelvin-worktree/*.log`. Wake contrast depends strongly on view and sun
  reflection; the field plot isolates the physical wave pattern from wind and
  shading. These captures do not measure the rendered envelope angle or imply
  rider acceptance of its appearance.
- Normal first-person and chase previews also close normally after capturing a
  running synthetic session at **10.15 m**, with the four metrics, missing HR
  shown as `--`, the +/-8 cm steering gauge and the existing distant scenery.
  Evidence: `artifacts/kelvin-worktree/row-view.png`, `artifacts/kelvin-worktree/row-wake.png` and corresponding
  logs. The copied map and water-material asset hashes match the training
  checkout; no generated scenery or material was changed.

These automated checks did not measure stereo HMD comfort, a long training
session or sustained UE frame time. The finite local deep-water model's physical and
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
