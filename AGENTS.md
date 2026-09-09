# Arrietty-row

Read README.md and docs/HANDOFF.md before changing the project.
Independent MIT C++ UE 5.8 rowing application. Do not modify sibling projects.
Water reference elevation is Z=0 cm. Preserve Lake Bled distant geometry.
Enter starts/pauses, numpad 0 stops/returns to spawn. HMD lateral position
steers; head yaw must never steer after calibration. Keep the camera horizon
level; animate boat and water separately. Missing heart rate is not zero bpm.
Start/resume uses 2 s settle, 1 s averaged quiet neutral and two bar strokes to
measure the machine axis. Calibration never advances the boat/session metrics.
Extra Enter during calibration preserves progress; NUM 0 cancels, Enter retries.
Keep the +/-8 cm straight zone and its gauge consistent; do not silently fall
back to gaze-based steering or continuously recenter during exercise.
Keep hardware identities, health/session data, generated UE content, Epic code,
and third-party binaries out of Git. BLE is telemetry only: no resistance or
fitness-machine control point writes. Never use another device as a silent
substitute for the configured Tracker. Record test evidence and limits honestly.
