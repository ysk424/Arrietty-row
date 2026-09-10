# Third-party notices

- Application code: MIT, Copyright (c) 2026 ysk424.
- Five selected sound masters were supplied by the rider as final Adobe Firefly
  output. Original WAVs and generated UE audio stay local;
  this repository does not assign them the application's MIT license.
  See [sound provenance](sounds/README.md).
- Instrument exposure setup adapts the MIT Arrietty-UE58 project, same copyright.
- OpenVR SDK: Valve Corporation, BSD-3-Clause. `tools/bootstrap.ps1` downloads
  only the pinned files in `tools/openvr.lock.json`, verifies SHA-256, and keeps
  the SDK outside Git. Its LICENSE is also staged alongside built binaries.
  [Valve SDK](https://github.com/ValveSoftware/openvr).
- Unreal Engine is separately licensed by Epic Games; engine code and binaries
  are not part of this MIT repository.
- Optional Python diagnostics use Bleak (MIT) and pyopenvr (MIT); they are not
  required by the C++ runtime.
- Lake Bled world assets have separate geographic data rights. See
  [scenery provenance](docs/SCENERY.md). No Secret-World GPL code or manekko
  Apache code is copied into this application.

Protocol references: [Bluetooth Fitness Machine Service 1.0](https://www.bluetooth.com/specifications/specs/fitness-machine-service-1-0/)
section 4.8; [Windows GATT client](https://learn.microsoft.com/en-us/windows/apps/develop/devices-sensors/gatt-client);
[Valve pose API](https://github.com/ValveSoftware/openvr/wiki/IVRSystem%3A%3AGetDeviceToAbsoluteTrackingPose).
