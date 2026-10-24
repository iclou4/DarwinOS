# Руководство разработчика darwinOS (Архитектура и Сборка)

В этом документе описывается внутреннее устройство оконной подсистемы, ядра, инструментария сборки и отладки darwinOS.

---

## 1. Стек оконной подсистемы (SkyLight WindowServer)

Оконный сервер (`CoreServices/SkyLight/WindowServer.c`) функционирует как изолированный системный сервис Mach (`WINDOWSERVER_SVC_NAME`), регистрируемый в `bootstrap_port`.

### Ключевые компоненты:
1. **Обработка Mach RPC:**
   - Каждое окно AppKit регистрируется через сообщение `kWSWindowCreate`.
   - Поверхность окна (`w->surface`) размещается в разделяемой памяти POSIX (`shm_open`), что позволяет процессу приложения напрямую отрисовывать контент без лишних межпроцессных копирований памяти.
2. **Оконный композитор (Metal 2D Pipeline):**
   - Композитинг использует ускоренный Metal-пайплайн (`Frameworks/Metal/src/mtl_raster.c`) для текстурированных 2D-квадов.
   - Поддерживает scissor-rect обрезку, чтобы перерисовывать только изменившиеся участки экрана (dirty bounding box) при перетаскивании.
   - Скругление углов $R=16$ осуществляется с субпиксельным альфа-сглаживанием, сохраняя фоновые пиксели нижних окон.
3. **Подсистема курсоров:**
   - Векторные SVG-ресурсы скомпилированы в `cursor_data.h` в формате 32-битного BGRA.
   - Отрисовка курсора сопровождается сохранением подложки `g_under`, благодаря чему перемещение указателя мыши не требует перекомпозитинга всей сцены и выполняется с частотой более 1000 FPS.

---

## 2. Сборка ядра и подсистем

### Необходимые инструменты:
- Кросс-компилятор Clang (`x86_64-apple-darwin19`) и линкер `ld64`.
- ravynOS SDK (`Developer/SDKs/ravynOS.sdk`).
- Python 3.8+ для скриптов пакетирования.

### Порядок сборки:
1. **Сборка Metal.framework:**
   ```bash
   bash /mnt/c/Users/antonov/AppData/Local/Temp/opencode/rebuild_libmetal2.sh
   ```
2. **Сборка WindowServer:**
   ```bash
   python3 /mnt/c/Users/antonov/AppData/Local/Temp/opencode/build_windowserver2.py
   bash /mnt/c/Users/antonov/AppData/Local/Temp/opencode/install_ws2_static.sh
   ```
3. **Сборка системных приложений (Chords, Installer):**
   ```bash
   python3 /mnt/c/Users/antonov/AppData/Local/Temp/opencode/build_thirdparty.py
   ```
4. **Пакетирование и развёртывание в образ диска (disk.img):**
   ```bash
   python3 /mnt/c/Users/antonov/AppData/Local/Temp/opencode/package_macos_system.py
   bash /mnt/c/Users/antonov/AppData/Local/Temp/opencode/deploy_metal.sh
   ```

---

## 3. Отладка и мониторинг

- **Kernel & Userland Logs:** Выводятся в последовательный порт QEMU `serial.log`.
- **Телеметрия производительности:**
  WindowServer периодически логирует длительность этапов композитинга:
  ```
  [USERLAND] comp_detail: setup=0.2ms commit=7.5ms cur=0.1ms total=7.8ms
  ```
- **Снятие скриншотов через QEMU Monitor (порт 4444):**
  ```powershell
  $tcp = New-Object System.Net.Sockets.TcpClient('127.0.0.1', 4444)
  $writer = New-Object System.IO.StreamWriter($tcp.GetStream()) { AutoFlush = $true }
  $writer.WriteLine('screendump screenshot.ppm')
  $tcp.Close()
  ```
