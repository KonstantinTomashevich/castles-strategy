# Castles Strategy
Prototype RTS game with simple mechanics.

## How to build
Step 1. Build [Urho3D](https://urho3d.github.io) SDK: clone sources from [git repository](https://github.com/Urho3D/Urho3D), [build it](https://urho3d.github.io/documentation/HEAD/_building.html) and generate SDK via `make install`.

Step 2. Set `URHO3D_HOME` environment var equal to the installed sdk dir.

Step 3. Clone repository:
```bash
https://github.com/KonstantinTomashevich/castles-strategy.git
```
Step 4. Generate build directory via CMake:
```bash
cd castles-strategy && mkdir build && cd build && cmake ..
```
Step 5. Build and run tests.
```bash
make && make test
```

## Controls
* WASD -- move camera.
* Click on tower to select it.
* Recruit and spawn units via top bar.
