#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
if [[ "$(uname -s)" != Darwin ]]; then
  echo 'This installer is for macOS. Build native targets for your platform first.'; exit 1
fi
bread_built='build/Bread_artefacts/Release'
bread_vst="$HOME/Library/Audio/Plug-Ins/VST3"
bread_au="$HOME/Library/Audio/Plug-Ins/Components"
for bread_file in "$bread_built/VST3/BREAD.vst3" "$bread_built/AU/BREAD.component"; do
  if [[ ! -d "$bread_file" ]]; then echo "Missing $bread_file. Run bash build-native.sh first."; exit 1; fi
done
if [[ -e "$bread_vst/BREAD.vst3" || -e "$bread_au/BREAD.component" ]]; then
  echo 'An installed BREAD plugin already exists. Move the old BREAD plugin aside before installing this build.'; exit 1
fi
mkdir -p "$bread_vst" "$bread_au"
ditto "$bread_built/VST3/BREAD.vst3" "$bread_vst/BREAD.vst3"
ditto "$bread_built/AU/BREAD.component" "$bread_au/BREAD.component"
echo 'Installed BREAD VST3 and AU. Restart your DAW and rescan plugins.'
