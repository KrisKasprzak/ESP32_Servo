# Changelog

All notable changes to this project are documented in this file.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Versions use the form `major.minor`: the major version changes only for
significant releases, and the minor version increases with every change.

## [1.2] - 2026-10-25

### Changed
- Header revision histories and copyright lines now credit "Kasprzak/Claude Code".

### Added
- SG90 servo photo in the README.

## [1.1] - 2026-10-25

### Added
- `PositionalServo`: instant `write()`, speed-limited non-blocking `moveTo()`,
  timed moves, easing profiles, blocking moves, software limits, trim, reverse.
- `ContinuousServo`: speed in percent, acceleration ramps, `runFor()` timed
  runs, `softStop()`, neutral calibration, dead band, reverse.
- `ServoBase`: LEDC handling for Arduino-ESP32 core 2.x and 3.x.
- Per-header version defines (`ESP32_SERVO_LIBRARY_VERSION`,
  `SERVOBASE_LIBRARY_VERSION`, `POSITIONALSERVO_LIBRARY_VERSION`,
  `CONTINUOUSSERVO_LIBRARY_VERSION`), MIT license text and revision history
  in every header.
- Examples: BasicSweep, BackAndForth, PotentiometerControl, SpeedAndEasing,
  ContinuousRotation, MultiServo.
