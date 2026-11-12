# OfficialCraft C++ 3.4.0 — Linux

Native C++17 / Qt 6 Minecraft Java launcher for Linux. Not affiliated with Mojang, Microsoft, Modrinth, or CurseForge.

## What's included

- Instance-based launcher with separate worlds, mods, resource packs, shader packs, versions, libraries, assets, and natives.
- Mojang version manifest, game-file download/repair, per-file download queue, SHA-1 verification, and offline launch support.
- Forge installer integration, including legacy Forge launcher-profile compatibility.
- Built-in **Modrinth mod browser**: search by name, filter to the selected Minecraft version and Forge, and install compatible releases without entering an API key.
- Attempts to install required Modrinth dependencies before the selected mod; validates downloaded JAR archives and SHA-1 hashes when supplied.
- Local mod manager: import JARs, enable/disable, remove, refresh, and open the selected instance's mods folder.
- Safe Java runtime discovery: checks the configured executable, PATH, JAVA_HOME, common Linux JDK directories, SDKMAN, mise, and local JDK folders.
- Reads Minecraft metadata `javaVersion`, checks JVM argument compatibility, prefers an exact Java major version, and refuses to launch with an older incompatible runtime.
- Java Auto-detect and Browse controls; Forge installer uses a compatible Java runtime too.
- Clean launch wrapper to avoid inherited Snap/GTK library conflicts on affected Ubuntu desktops.

## Mod downloads — no API key

The mod browser uses Modrinth's public API. Modrinth documents that most read-only API requests do not require authentication. OfficialCraft does not ask for, store, or send an API key. The old CurseForge API-key field and authenticated CurseForge download code have been removed. This release downloads mods from Modrinth only; it does not scrape CurseForge pages or bypass CurseForge's access controls.

Mod search and version selection are filtered to the Minecraft version selected for the instance and Forge. The Forge profile must already be installed and selected before installing Forge mods. The downloader selects the primary file (or first file if none is marked primary), verifies SHA-1 where available, checks that the downloaded file is a ZIP/JAR archive, and installs required dependencies when Modrinth provides compatible dependency metadata.

## Ubuntu dependencies

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6ct openjdk-8-jre openjdk-17-jre openjdk-21-jre unzip
```

Java is discovered automatically. Install the runtime major version required by the selected Minecraft metadata; older releases such as Minecraft 1.12.2 generally require Java 8, while modern releases commonly require Java 21 or newer. The launcher will display the versions it finds if no compatible runtime is installed.

## Build and run

Extract the ZIP, then from the project directory:

```bash
chmod +x build.sh run.sh
JOBS=1 ./build.sh
./run.sh
```

Use `./run.sh` rather than launching `./build/officialcraft` directly on systems where Snap/GTK variables leak into Qt processes. The wrapper clears inherited library/plugin variables while preserving the display session.

## Instance data

Data is stored under Qt's per-user application data directory, normally `~/.local/share/OfficialCraft/officialcraft/`:

```text
instances/<instance>/
  game/{saves,mods,resourcepacks,shaderpacks}/
  versions/<version>/
  libraries/
  assets/{indexes,objects}/
  natives/
  logs/
  instance.json
```

**Do not delete the `instances` directory when updating the launcher**; it contains your worlds, installed mods, and game data. Replace the source files and rebuild only the `build/` directory.

## Current limitations

- Microsoft OAuth / Xbox / Minecraft Services authentication is not implemented. Launch is offline-only; authenticated multiplayer and Realms are unavailable.
- The in-app mod browser currently supports Forge mods from Modrinth. Fabric, NeoForge, and Quilt installers/browsing are not included in this release.
- CurseForge integration and CurseForge modpack import are not included.
- Modrinth projects may have external dependencies or optional requirements that cannot be installed automatically. Review a mod's requirements before launching.
- Download batches run in parallel, but there are no pause/resume controls or persistent queue recovery yet.
- Automatic Java discovery searches standard Linux locations but does not download or install JDKs. If the required runtime is missing, install it with your distribution package manager and press Auto-detect.
- The launch argument builder remains basic and is not fully compatible with every modern Minecraft release.
- This source revision could not be compiled in the generation environment because Qt 6 development packages are unavailable there. Build and test it on the target Ubuntu PC before relying on it.


## Java runtime discovery

Each instance may use `auto` (recommended) or an explicit executable path. On launch, OfficialCraft inspects installed Java executables with `java -version`, reads `javaVersion.majorVersion` from Minecraft version metadata, and checks known JVM flags. It searches PATH, JAVA_HOME, `/usr/lib/jvm`, `/usr/java`, `/usr/local/java`, `/opt/java`, `/opt/jdk`, `~/.jdks`, SDKMAN, and mise installations. It prefers an exact Java major match and will not choose an older runtime than the metadata requires. The launcher does not run shell commands from metadata and does not silently rewrite JVM arguments.
