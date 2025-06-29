# Guidelines for Codex

## Building
- Run `apt-get update` and `apt-get install -y libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev` before compiling.
- Use `make` to build the `game` executable.

## Running
- Run `./game`. The program expects `assets/room_000.png` to exist.
- A valid X11/Wayland session is required; running inside headless environments may fail with `XDG_RUNTIME_DIR` errors.
