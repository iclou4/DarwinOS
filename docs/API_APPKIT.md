# Руководство по разработке приложений для darwinOS (AppKit API)

Приложения в darwinOS разрабатываются с использованием нативного Objective-C фреймворка **AppKit** и среды **Foundation**, полностью соответствуя парадигме Apple Cocoa.

---

## 1. Структура AppKit-приложения

Типичный жизненный цикл приложения состоит из инициализации `NSApplication`, делегата приложения `NSApplicationDelegate` и создания главного окна `NSWindow`.

```objc
#import <AppKit/AppKit.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    NSRect frame = NSMakeRect(200, 200, 640, 480);
    NSUInteger style = NSWindowStyleMaskTitled |
                       NSWindowStyleMaskClosable |
                       NSWindowStyleMaskMiniaturizable |
                       NSWindowStyleMaskResizable;

    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:style
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];

    [self.window setTitle:@"Моё приложение darwinOS"];
    [self.window makeKeyAndOrderFront:nil];
}

@end

int main(int argc, const char *argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *delegate = [[AppDelegate alloc] init];
        [app setDelegate:delegate];
        [app run];
    }
    return 0;
}
```

---

## 2. Стилизация окон: macOS 26 Tahoe Chrome

В darwinOS окна автоматически отрисовываются с современным стилем:
- **Высота заголовка:** 32 пикселя.
- **Радиус скругления:** $R=16$ пикселей на всех четырёх углах.
- **Traffic Lights (Кнопки управления):**
  - Красная: Закрытие `[window close]`
  - Жёлтая: Сворачивание `[window miniaturize:]`
  - Зелёная: Масштабирование / Fullscreen `[window zoom:]`
- **Непрерывные мягкие тени:** Реализованы непрерывным Signed Distance Field (SDF) алгоритмом, мягко затемняющим нижележащие окна и рабочий стол.

---

## 3. Системные указатели мыши (`NSCursor`)

darwinOS поддерживает полный набор системных указателей мыши в соответствии с macOS:

| Вызов в коде | Тип курсора | Описание |
| :--- | :--- | :--- |
| `[NSCursor arrowCursor]` | Стрелка | Стандартный указатель системы |
| `[NSCursor IBeamCursor]` | Текстовый I-Beam | Выделение текста в полях ввода |
| `[NSCursor pointingHandCursor]` | Рука с пальцем | Ссылки, интерактивные кнопки |
| `[NSCursor openHandCursor]` | Открытая рука | Панорамирование, захват холста |
| `[NSCursor closedHandCursor]` | Кулак | Перетаскивание объектов |
| `[NSCursor resizeLeftRightCursor]` | Горизонтальный ресайз | Изменение ширины колонок/окон |
| `[NSCursor resizeUpDownCursor]` | Вертикальный ресайз | Изменение высоты панелей/окон |
| `[NSCursor crosshairCursor]` | Перекрестие | Графические редакторы, прицел |
| `[NSCursor busyCursor]` / `waitCursor` | Beachball | Индикатор ожидания (анимация) |

### Пример установки курсора:

```objc
- (void)mouseEntered:(NSEvent *)event {
    [[NSCursor pointingHandCursor] push];
}

- (void)mouseExited:(NSEvent *)event {
    [NSCursor pop];
}
```

---

## 4. Отрисовка графики (`NSView` и `CoreGraphics`)

Для создания пользовательского отображения переопределяется метод `drawRect:`:

```objc
@interface CustomView : NSView
@end

@implementation CustomView

- (void)drawRect:(NSRect)dirtyRect {
    CGContextRef ctx = [[NSGraphicsContext currentContext] CGContext];

    // Рисование градиента
    CGContextSetRGBFillColor(ctx, 0.15, 0.55, 0.95, 1.0);
    CGContextFillRect(ctx, dirtyRect);

    // Рисование скругленного прямоугольника
    CGContextSetRGBFillColor(ctx, 1.0, 1.0, 1.0, 0.9);
    CGRect rounded = CGRectMake(20, 20, 200, 100);
    CGPathRef path = CGPathCreateWithRoundedRect(rounded, 12, 12, NULL);
    CGContextAddPath(ctx, path);
    CGContextFillPath(ctx);
    CGPathRelease(path);
}

@end
```

---

## 5. Компиляция и сборка приложения

Сборка приложений выполняется с флагом `-framework AppKit -framework Foundation -framework CoreGraphics`:

```bash
clang -target x86_64-apple-darwin19 \
  --sysroot=/Library/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk \
  -mmacosx-version-min=10.15 \
  -O2 -pthread \
  -framework AppKit \
  -framework Foundation \
  -framework CoreGraphics \
  -framework Onyx2D \
  -lobjc -lSystem \
  Source/main.m -o MyApp.app/Contents/MacOS/MyApp
```
