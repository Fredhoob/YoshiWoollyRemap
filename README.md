# Yoshi's Woolly World — Wii Remote B acts as A (Aroma/WUPS)

**Status: source project; NOT console-tested or compiled to `.wps` in the authoring environment.**

## Behavior

When the native Wii U **Yoshi's Woolly World** title is running, a Wii Remote
with **no Nunchuk/Classic attachment** behaves like this:

- Physical **A** = Wii Remote A (unchanged).
- Physical underside **B** = Wii Remote A (B's original function is suppressed).
- 1, 2, D-pad, +, -, Home = unchanged.
- Wii U GamePad, Pro Controller, Classic Controller, Nunchuk = unchanged.
- All other games = unchanged.
- Both Wii Remotes in local co-op are affected (when used without extensions).

The plugin recognizes the game's regional IDs:

| Region | Title ID |
| --- | --- |
| Japan | `0005000010131F00` |
| USA | `0005000010184D00` |
| Europe | `0005000010184E00` |

**Sideways limitation:** Hardware does not expose a reliable *sideways-mode*
flag. This is restricted to **bare Wii Remotes in Woolly World**, which is the
horizontal Wii Remote mode in the game. It doesn't try to infer angle from
noisy accelerometer values.

## Build on a computer with Docker

```bash
docker build -t yoshi-wps-builder .
docker run --rm -v "$PWD:/project" -w /project yoshi-wps-builder make
```

Output: `yoshi_woollyworld_wiimote_b_as_a.wps`.

Or upload this project as a GitHub repo, select **Actions → Build Aroma WPS →
Run workflow**, then download the `.wps` from the successful run's artifact.

The build requires the Wii U **devkitPPC/wut and WUPS toolchain**. The included
Dockerfile pins the upstream containers used by a working public WUPS plugin.
No third-party game files are needed.

### Windows PowerShell with Docker Desktop

From the extracted project folder:

```powershell
docker build -t yoshi-wps-builder .
docker run --rm -v "${PWD}:/project" -w /project yoshi-wps-builder make
```

If Docker is unavailable, the included GitHub Actions workflow can build the
`.wps` on a hosted Linux runner instead.

## Install (after a successful build)

1. Copy `yoshi_woollyworld_wiimote_b_as_a.wps` to
   `sd:/wiiu/environments/aroma/plugins/`.
2. Boot Aroma. Do not install it to NAND or modify the game files.
3. Start Yoshi's Woolly World, choose horizontal Wii Remote controls, and test
   B on a simple, safe level before relying on it.
4. To roll back, power off, remove the `.wps` from that SD folder, and reboot.

## Compatibility caveats

The source hooks four `padscore.rpl` read APIs (`KPADRead`, `KPADReadEx`,
`KPADGetUnifiedWpadStatus`, `WPADRead`), following patterns from
`jgardia96/classic-con-wiiu`. These APIs are plausible for the game, but the
actual calls made by Yoshi's Woolly World have **not been verified on hardware**.
The game might read input through another route. A test on the actual Wii U is
necessary before calling this a working mod. If there are conflicting Aroma
input plugins, disable those temporarily for diagnosis.

## Sources

- WUPS: https://github.com/wiiu-env/WiiUPluginSystem
- wut padscore API: https://github.com/devkitPro/wut
- Reference plugin: https://github.com/jgardia96/classic-con-wiiu
- Title database: https://wiiubrew.org/wiki/Title_database

## Local pure logic test

```bash
g++ -std=c++20 -Wall -Wextra -pedantic tests/test_remap.cpp -o test_remap
./test_remap
```

This only validates the button transformation and does **not** test WUPS
integration or Wii U behavior.
