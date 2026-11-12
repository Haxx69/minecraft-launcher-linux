#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$(readlink -f "$0")")"
# Isolate the Qt app from inherited Snap/GTK library variables on some Ubuntu desktops.
exec env -i \
  HOME="$HOME" USER="${USER:-$(id -un)}" \
  PATH="/usr/local/bin:/usr/bin:/bin" \
  DISPLAY="${DISPLAY:-}" XAUTHORITY="${XAUTHORITY:-}" \
  XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-}" \
  DBUS_SESSION_BUS_ADDRESS="${DBUS_SESSION_BUS_ADDRESS:-}" \
  WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-}" \
  QT_QPA_PLATFORM=xcb QT_QPA_PLATFORMTHEME=qt6ct \
  ./build/officialcraft "$@"
