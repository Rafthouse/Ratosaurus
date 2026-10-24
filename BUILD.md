# Збірка з вихідного коду

Ratosaurus — компресор-дістресор (VST3 + Standalone). JUCE 8.0.13 завантажується автоматично через FetchContent.

## Автоматично (GitHub Actions)

1. При кожному `git push` у `main` воркфлоу `.github/workflows/cross-platform-builds.yml` збирає Windows, macOS і Linux, пакує інсталятори (Inno Setup `.exe` + `.pkg`/`.dmg`).
2. По тегу `v*` артефакти потрапляють у GitHub Release.

## Локально — Windows (Visual Studio 2022 + CMake)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Готовий VST3: `build\Ratosaurus_artefacts\Release\VST3\Ratosaurus.vst3`.

## Локально — macOS (Xcode CLT + CMake)

```bash
cmake -S . -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build --config Release --parallel
codesign --force --deep --sign - "build/Ratosaurus_artefacts/Release/VST3/Ratosaurus.vst3"
```

## Локально — Linux

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target Ratosaurus_VST3 --parallel
```

## Інсталятори

- **Windows:** скрипт `installer/ratosaurus.iss` (Inno Setup) → `.exe`.
- **macOS:** у воркфлоу — `pkgbuild` (`.pkg`) та `create-dmg` (`.dmg`).

## Ім'я автора / бренд

Автор задається у `Source/PluginProcessor.cpp` та через COMPANY_NAME у `CMakeLists.txt`.
