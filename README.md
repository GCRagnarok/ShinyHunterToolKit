# ShinyHunterToolKit

## Overview
ShinyHunterToolKit is a tool for shiny hunting in Pokémon games on emulators. It uses ImGui for the user interface, GLFW for window management, GLEW for OpenGL extension loading, and ViGEm for virtual gamepad emulation.

Only supports XInput devices, i.e., Xbox controllers.

## Features
### Shiny Counter:
- Set the generation you are currently hunting in.
- Set the number of encounters per reset (multiple game instances).
- Set the current number of encounters.
- Manually increment the counter using the GUI button.
- Automatically increment the counter when pressing the reset combo for the selected generation. (i.e START/SELECT + LB + RB - generations 6-7)

### Controller Manager:
- Display physical and virtual controller status (text colour green/red = connected/disconnected)
- Repeated virtual controller button presses:
  1. Use the GUI button to engage.
  2. Press any button on the physical controller.
  3. Virtual controller will repeatedly press this button.
  4. Use the GUI button to disengage.
- Record/Playback Macros:
    1. Use the GUI button or controller combo (hold LT + RT for 1 second) to begin recording.
    2. Press any sequence of buttons on the physical controller.
    3. Use the GUI button or controller combo (hold LT + RT for 1 second) to finish recording.
    4. Use the GUI button or controller combo (L3 + R3) to playback recording.
    5. Virtual controller will playback recording in a loop.
    6. Use the GUI button or controller combo (L3 + R3) to stop playback.

## Download
- Head to [Releases](https://github.com/GCRagnarok/ShinyHunterToolKit/releases) and download the latest release (ShinyHunterToolKit_vX.X).
- Unzip and run ShinyHunterToolKit.exe.
