# Serial to Wifi
Firmware that lets you pipe serial data from your pico 2w or w to a client connected over a pre-configured port as a TCP server.

## Usage
Add your credentials to 'src/config.hpp', then build and drop the uf2 file onto your pico. When the pico starts it will attempt to connect to the router immediately and wait for an incoming client to connect to it.

## Building
Run 'cmake -S . -B build -DPICO_BOARD=pico2_w' to generate the build dir. Make sure you have 'PICO_SDK_PATH' pointing to your pico_sdk path.
