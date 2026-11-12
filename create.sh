sudo apt update
sudo apt install -y file wget patchelf desktop-file-utils squashfs-tools

cd ~/projects/coding/mc-launcher
JOBS=1 ./build.sh

cd ~/projects/coding/mc-launcher

wget -O linuxdeploy-x86_64.AppImage \
https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage

wget -O linuxdeploy-plugin-qt-x86_64.AppImage \
https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage

chmod +x linuxdeploy*.AppImage

mkdir -p packaging

cat > packaging/officialcraft.desktop <<'EOF'
[Desktop Entry]
Name=OfficialCraft
Comment=Minecraft Launcher
Exec=officialcraft
Icon=officialcraft
Terminal=false
Type=Application
Categories=Game;
EOF

cat > packaging/officialcraft.svg <<'EOF'
<svg xmlns="http://www.w3.org/2000/svg" width="256" height="256" viewBox="0 0 256 256">
<rect width="256" height="256" rx="40" fill="#252b45"/>
<path d="M128 28 218 78 218 178 128 228 38 178 38 78Z" fill="#4f83e8"/>
<path d="M128 28 218 78 128 128 38 78Z" fill="#8db7ff"/>
<path d="M128 128 218 78 218 178 128 228Z" fill="#3563bd"/>
<path d="M38 78 128 128 128 228 38 178Z" fill="#4778d5"/>
<path d="M92 93h72v70H92z" fill="#fff"/>
<path d="M110 108h36v40h-36z" fill="#252b45"/>
</svg>
EOF

cd ~/projects/coding/mc-launcher

export QMAKE=/usr/lib/qt6/bin/qmake

APPIMAGE_EXTRACT_AND_RUN=1 \
LDAI_OUTPUT=OfficialCraft-3.3.1-x86_64.AppImage \
./linuxdeploy-x86_64.AppImage \
  --appdir AppDir \
  --executable build/officialcraft \
  --desktop-file packaging/officialcraft.desktop \
  --icon-file packaging/officialcraft.svg \
  --plugin qt \
  --output appimage


