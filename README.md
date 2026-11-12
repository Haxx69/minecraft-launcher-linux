# OfficialCraft

A lightweight Minecraft launcher for Linux, designed to keep the launcher simple and minimize unnecessary overhead.

> **Unofficial project:** OfficialCraft is not affiliated with, endorsed by, or sponsored by Mojang or Microsoft. Minecraft is a trademark of Mojang Synergies AB.
>
> Use Minecraft and any offline-mode functionality only in accordance with the game's terms and the rights to the game files you use. This project does not include Minecraft game files or provide a Minecraft license.

## Features

- Lightweight Linux launcher.
- Java Edition support.
- Build an AppImage locally from the project source.
- Uses `linuxdeploy` and its Qt plugin to package the application.

## Build on Linux

### Requirements

- A Linux distribution with a supported package manager (`apt`, `dnf`, `pacman`, or `zypper`).
- The project source code, including `build.sh`.
- An internet connection for downloading the packaging tools.
- A working Qt 6 development environment.

### Build

1. Download or clone the complete project source.
2. Open a terminal in the project directory.
3. Make the build script executable:

   ```bash
   chmod +x ./create.sh
   ```

4. Run it:

   ```bash
   ./create.sh
   ```

The script installs the required system packages (after asking for confirmation), runs the project's `build.sh`, downloads `linuxdeploy` and its Qt plugin, and attempts to package the application as an AppImage.

The expected output filename is:

```text
OfficialCraft-3.3.1-x86_64.AppImage
```

The AppImage is intended for **x86_64** Linux systems.

## Build configuration

The script uses `JOBS=1` by default to keep build resource usage low. To override this, run:

```bash
JOBS=4 ./create.sh
```

Change `4` to the number of parallel build jobs you want to use.

## Troubleshooting

- **`build.sh` not found:** Make sure you downloaded the complete source repository and run `create.sh` from that project.
- **Qt / qmake not found:** Install the Qt 6 development packages for your distribution and ensure `qmake6` or Qt 6's `qmake` is available.
- **Build fails:** Read the first error printed by `build.sh`; the packaging step cannot work unless the application binary is created at `build/officialcraft`.
- **AppImage does not start:** Some Linux distributions may need additional FUSE compatibility packages. You can also try the AppImage's supported extraction/run options.

## Project status and contributions

Issues, bug reports, and improvements are welcome. Please include your Linux distribution, architecture, and the relevant build or runtime error when reporting a problem.

## Disclaimer

This repository provides launcher source code and build instructions. It does not distribute Minecraft, game assets, account credentials, or a license. You are responsible for complying with applicable laws and the Minecraft End User License Agreement.
