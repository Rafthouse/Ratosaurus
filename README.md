# Ratosaurus

**Компресор-дістресор** (compressor-distressor) у форматі VST3 + Standalone з характеровим насиченням. Формати: **VST3 та Standalone** для Windows, macOS (Universal Binary: Apple Silicon M1–M4 + Intel) та Linux.

Автор: **Mykyta Shchur**

---

## ⬇️ Завантажити та встановити

### Windows (VST3 + Standalone)

| Файл | Опис |
|---|---|
| [Ratosaurus-Windows-Setup-x64.exe](https://github.com/Rafthouse/Ratosaurus/releases/download/v1.0.0/Ratosaurus-Windows-Setup-x64.exe) | Інсталятор — ставить VST3 у систему + Standalone |

1. Скачайте `.exe` і запустіть.
2. Інсталятор покладе VST3 у `C:\Program Files\Common Files\VST3\` і Standalone у `Program Files\Ratosaurus Audio\Ratosaurus\`.
3. Перезапустіть DAW (Reaper, Ableton, Cubase, FL Studio).

### macOS (VST3 + Standalone)

| Файл | Опис |
|---|---|
| [Ratosaurus-macOS-Universal2.pkg](https://github.com/Rafthouse/Ratosaurus/releases/download/v1.0.0/Ratosaurus-macOS-Universal2.pkg) | Інсталятор — ставить VST3 у системну папку |
| [Ratosaurus-macOS-Universal2.dmg](https://github.com/Rafthouse/Ratosaurus/releases/download/v1.0.0/Ratosaurus-macOS-Universal2.dmg) | Disk image — перетягніть плагін вручну |

1. Скачайте `.pkg` (найпростіше) або `.dmg`.
2. Для `.pkg`: двічі клацніть → Installer → далі.
3. Для `.dmg`: перетягніть `Ratosaurus.vst3` у `~/Library/Audio/Plug-Ins/VST3/`.
4. Перезапустіть DAW.

> **Перший запуск на macOS:** збірки підписані ad-hoc. Якщо Gatekeeper скаржиться — правий клік → **Відкрити**, або `xattr -cr /шлях/до/файлу`.

### Linux (VST3)

Артефакт `Ratosaurus-Linux-x86_64-VST3.tar.gz` лежить у Release. Розпакуйте і покладіть `Ratosaurus.vst3` у `~/.vst3/`.

---

## 🎛️ Як користуватися (параметри)

| Параметр | Діапазон | Що робить |
|---|---|---|
| **Bypass** | ON/OFF | Обхід обробки |
| **Input** | −60…+12 dB | Вхідний рівень |
| **Output** | −60…+12 dB | Вихідний рівень (make-up gain) |
| **Threshold** | −60…0 dB | Поріг компресії |
| **Ratio** | 1:1…20:1 | Ступінь стиснення |
| **Attack** | 0.05…50 ms | Швидкість спрацювання |
| **Release** | 10…1000 ms | Час відновлення |
| **Detector** | Peak / RMS | Тип детектора |
| **Knee** | Hard / Medium / Soft | М'якість коліна компресії |
| **Character** | Clean / Punch / Rat | Характер обробки |
| **Drive** | 0…1 | Рівень підсилення сатурації |
| **Density** | 0…1 | Щільність гармонік |
| **Color** | −1…1 | Тональне забарвлення |
| **Saturation** | Off / Tape / Tube / FET / Rat | Модель сатурації |
| **Oversampling** | Off / 2x / 4x / 8x | Оверсемплінг для чистоти |
| **VU Calibration** | −18/−24/−20 dBFS | Калібрування VU-метра |

### Швидкі рецепти

- **М'яка компресія вокалу:** Ratio 3:1, Threshold ≈ −24 dB, Attack 10 ms, Release 150 ms, Detector = RMS.
- **Агресивний дістресор:** Character = Rat, Saturation = FET, Drive 0.6, Ratio 8:1.
- **Теплий «клей» на шину:** Saturation = Tube, Oversampling 4x, Density 0.5, Ratio 2:1.

---

## 📦 Формати та вимоги

- **Windows:** VST3 (x64) + Standalone, Windows 10/11.
- **macOS:** VST3 + Standalone, Universal Binary (arm64 + x86_64), macOS 11+.
- **Linux:** VST3 (x86_64).
- **DAW:** будь-які з підтримкою VST3 — Reaper, Ableton Live, Cubase, Studio One, FL Studio, Bitwig, Logic Pro (macOS).

---

## 🔧 Збірка з вихідного коду

Інструкція локальної збірки та CI — у [BUILD.md](BUILD.md).
