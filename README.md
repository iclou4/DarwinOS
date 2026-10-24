# darwinOS

<p align="center">
  <img src="https://raw.githubusercontent.com/iclou4/DarwinOS/main/CoreServices/WindowServer/Cursors/arrowCursor.png" width="48" alt="darwinOS Logo" />
</p>

**darwinOS** — открытая операционная система, основанная на гибридном ядре **XNU** (Mach + BSD), графическом сервере **SkyLight WindowServer** и нативной среде исполнения **Cocoa / AppKit**. Проект нацелен на полную бинарную и исходную совместимость с macOS, современный визуальный дизайн macOS 26 Tahoe и работу как в гипервизорах (QEMU/KVM/WHPX), так и на реальном x86_64 оборудовании.

---

## 📑 Разделы документации (API & Guides)

- 📘 [**Базовая информация и запуск (README)**](README.md) — обзор системы, архитектура и запуск
- ⚙️ [**API разработки драйверов (IOKit & Kexts)**](docs/API_DRIVERS.md) — создание драйверов, IOKit, ApplePS2, графика и ввод
- 🎨 [**API разработки приложений (AppKit & Cocoa)**](docs/API_APPKIT.md) — создание графических окон, контролов, курсоров и событий
- 🛠️ [**Руководство разработчика (System Architecture & Build)**](docs/DEVELOPMENT.md) — сборка системы, ядро XNU, компилятор и отладка

---

## 🏛️ Архитектура системы

```mermaid
graph TD
    subgraph Userland ["Userland (Cocoa / AppKit Environment)"]
        Apps["Cocoa Applications (Install darwinOS, Chords, Finder)"]
        AppKit["AppKit.framework & Foundation.framework"]
        CG["CoreGraphics.framework & Onyx2D.framework"]
        Metal["Metal.framework (Accelerated UI Rasterizer)"]
        WS["SkyLight WindowServer (Compositor & Event Router)"]
    end

    subgraph IPC ["Mach IPC & Shared Memory"]
        MachMsg["Mach Messages (RPC Port 8000+)"]
        ShmSurfaces["POSIX Shared Memory Backing Stores"]
    end

    subgraph Kernel ["Kernel Space (XNU Kernel)"]
        MachKernel["Mach Microkernel (Threads, Tasks, IPC, VM)"]
        BSDKernel["BSD Subsystem (VFS, POSIX, Syscalls, Networking)"]
        IOKit["IOKit C++ Driver Model"]
        PS2Driver["ApplePS2Controller (Mouse, Keyboard)"]
        VRAM["VirtIO-VGA / Linear VRAM Framebuffer"]
    end

    Apps --> AppKit
    AppKit --> CG
    AppKit --> MachMsg
    CG --> Metal
    WS --> ShmSurfaces
    WS --> MachMsg
    MachMsg --> MachKernel
    ShmSurfaces --> BSDKernel
    IOKit --> PS2Driver
    IOKit --> VRAM
```

---

## 🚀 Основные особенности

1. **Ядро XNU x86_64:**
   - Полноценная поддержка многозадачности, вытесняющей многопоточности, Mach IPC и POSIX-вызовов.
   - Механизм единого prelinked `kernelcache` с встроенными kexts и init-payload.
2. **SkyLight WindowServer:**
   - Аппаратно-ускоренный Metal-композитор оконных слоёв в реальном времени.
   - macOS 26 Tahoe styling: скругление окон $R=16$, панель заголовка 32px, traffic lights (кнопки закрытия, сворачивания, масштабирования) и непрерывные мягкие тени (continuous SDF drop shadows).
   - Поддержка перекрывающихся окон (painter's order) с сохранением теней и субпиксельным сглаживанием углов.
3. **Системные курсоры macOS 26 Tahoe:**
   - 19 системных типов курсоров (`arrow`, `ibeam`, `pointinghand`, `openhand`, `closedhand`, `busy` / `beachball`, `crosshair`, `resize*`, `move`, `menu`, `zoomin`, `zoomout`).
   - Настоящая анимация радужного пляжного мяча (12 фаз вращения с частотой 80 мс).
   - Полная интеграция с `NSCursor` и `[NSCursor set]` через IPC.
4. **AppKit & CoreGraphics:**
   - Нативная поддержка `NSApplication`, `NSWindow`, `NSView`, `NSTextField`, `NSButton`, обработка событий `NSEvent`.

---

## ⚡ Быстрый старт (Запуск в QEMU)

Для запуска подготовленного образа в QEMU:

```bat
"C:\Program Files\qemu\qemu-system-x86_64.exe" ^
  -name "darwinOS" ^
  -m 8G -machine q35 -cpu Penryn -smp 4,cores=4,sockets=1 ^
  -accel tcg,thread=single,tb-size=1024 ^
  -drive if=pflash,format=raw,file=OVMF_CODE_4M.fd,read-only=on ^
  -drive if=pflash,format=raw,file=OVMF_VARS.fd ^
  -drive if=ide,id=disk0,file=disk.img,format=raw ^
  -device virtio-rng-pci,rng=rng0 ^
  -device virtio-vga,xres=1280,yres=800 ^
  -serial file:serial.log ^
  -display sdl
```

---

## 📜 Лицензия

Проект распространяется под лицензиями Apple Public Source License (APSL 2.0) и MIT License.
