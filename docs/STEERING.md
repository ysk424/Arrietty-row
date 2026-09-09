# Steering and start calibration

Every start/resume measures a new physical neutral and machine axis. The boat,
active exercise time, virtual distance and stroke count remain stopped during
setup. Extra keypad Enter presses preserve setup progress. Numpad 0 cancels
setup/stops/home; Enter then retries. Setup instructions follow the camera until
calibration succeeds, so they remain visible before a room neutral exists.
The instrument panel returns to its boat attachment when rowing begins.
No ongoing adaptive recentering changes the meaning of a deliberate lean.

1. **Get comfortable (2 s):** ignore the posture used to reach the keypad.
2. **Hold still (1 s):** average HMD position over a continuous quiet interval.
   A range above 3 cm on any position axis restarts that interval. Facing must
   have a usable horizontal component. This allows normal breathing/jitter.
3. **Row straight (two out-and-back strokes):** collect the horizontal bar
   trajectory and fit its principal axis. Gaze selects only which end of that
   line is forward; it never supplies the axis angle. A direction more than
   60 degrees from the facing captured in step 2 is not accepted.

Axis fit requirements: at least 4 s of motion, 40 cm total travel span, four
alternating excursions of at least 30 cm, horizontal variance at least
0.008 square metres along the major axis, and minor/major variance ratio at
most 0.04. Samples are distinct and capped at 120 Hz. A single direction of
travel, stationary noise or a broad circular gesture cannot complete setup.
The four excursions recognize two strokes before the final return endpoint;
the next normal stroke begins the exercise. The gauge switches on automatically.

Setup times out after 40 s total. Invalid/stale tracking, a frame gap above
100 ms or an implausible bar discontinuity cancels it. Retry is explicit; there
is no fallback to the old gaze-based axis. Calibration stays in memory and is
discarded on stop/home or a fresh setup. The offline test mode generates the
same setup motion and runs the same calibration path, without device access.

During exercise, lateral position is the dot product of HMD displacement from
the averaged neutral with the right vector perpendicular to the measured bar
axis. Therefore normal fore/aft movement along that axis does not steer, even
when the rider looked a little sideways during setup. UE maps that axis into
the rendered world by subtracting physical head yaw from rendered head yaw.

| Parameter | Value |
| --- | --- |
| Straight zone | -8 to +8 cm of filtered lateral position |
| Full steering | 26 cm left/right |
| Curve beyond the margin | Squared normalized displacement, gentle near center |
| Maximum turn rate | 0.20 rad/s (about 11.5 degrees/s) |
| Low speed scaling | Linear to full strength at 1.2 m/s |
| Position smoothing | Exponential, 0.28 s time constant |
| Turn smoothing outside the zone | Exponential, 0.35 s time constant |
| Return inside the zone | Turn rate becomes zero; current heading is preserved |

The gauge uses that same filtered position, with a green straight band and a
marker over a +/-30 cm scale. CENTER means no commanded turn; LEFT and RIGHT
indicate steering direction, whose effect also depends on speed. No valid
calibration/tracking means SETUP with no marker. The camera horizon remains
level; only the boat/water visuals bob. Private session CSV adds `lean_cm`,
`steer` (-1 to +1), and `yaw_deg_s` for diagnosing drift without guessing.
