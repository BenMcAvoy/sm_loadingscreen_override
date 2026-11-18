# sm_loadingscreen_override

This is a Windows C++ modding experiment for replacing Scrap Mechanic's loading-screen image. The DLL hooks a MyGUI loading-screen callback and points the image box at `gui_loadingscreen_axolotl.png`.

## Build

Install [xmake](https://xmake.io) and build the target from the repository root:

```sh
xmake
```

## Status

The implementation relies on hard-coded game addresses and object offsets, and the image filename is fixed in the source. It is specific to the game version it was developed against and needs updating when those layouts change.
