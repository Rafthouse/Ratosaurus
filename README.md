# Ratosaurus

Ratosaurus is a C++20/JUCE 8 compressor-distressor VST3 by Mykyta Shchur.

## Supported build targets

- Windows 11 x64: VST3 and Standalone
- macOS 11+: Universal VST3 (`arm64` + `x86_64`)
- Linux x86_64: VST3

## Local build

JUCE 8.0.13 is fetched automatically by CMake.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target Ratosaurus_VST3 --parallel
```

On multi-config generators such as Visual Studio or Xcode, add
`--config Release` to the build command.

## CI artifacts

The **Cross-platform VST3 builds** GitHub Actions workflow builds native Linux
and universal macOS bundles. Artifacts are attached to every successful run.
Pushing a tag such as `v1.0.0` creates a GitHub Release containing both builds.

## macOS distribution note

CI applies an ad-hoc signature so the bundle is structurally valid for local
testing. Public distribution without Gatekeeper warnings requires an Apple
Developer ID certificate and notarization credentials.
