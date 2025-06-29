# Suoranta SDL Example

This repository contains a minimal SDL2 program that toggles exclusive fullscreen mode at 320x200.

## Building

Install development libraries:
```bash
sudo apt-get update
sudo apt-get install -y libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```
Then compile with `make`.

Place `assets/room_000.png` in an `assets` folder next to the executable. Run the program:
```bash
./game
```
Note that it requires a graphical environment. Running without a display may produce `XDG_RUNTIME_DIR` errors.
