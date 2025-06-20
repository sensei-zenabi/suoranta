#!/usr/bin/env bash
set -e

echo "Updating package list…"
sudo apt-get update

echo "Installing build essentials and pkg-config…"
sudo apt-get install -y build-essential pkg-config

echo "Installing SDL2 development libraries…"
sudo apt-get install -y libsdl2-dev libsdl2-gfx-dev

echo "All dependencies installed!"
