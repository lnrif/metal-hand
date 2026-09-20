#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned int u32;

#define S 80
#define W (16 * S)
#define H (9 * S)

int main() {
    Display *d = XOpenDisplay(NULL);
    if (!d) return 1;

    int s = DefaultScreen(d);
    Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, W, H, 1, 0, 0);

    // Добавляем все нужные маски
    XSelectInput(d, w, KeyPressMask | ButtonPressMask | ButtonReleaseMask | 
                       PointerMotionMask | Button1MotionMask | StructureNotifyMask);
    
    // Регистрируем протокол удаления окна
    Atom wmDeleteMessage = XInternAtom(d, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(d, w, &wmDeleteMessage, 1);

    XMapWindow(d, w);

    u32 *pixels = calloc(W * H, sizeof(u32)); 
    XImage *image = XCreateImage(d, DefaultVisual(d, s), DefaultDepth(d, s),
                                 ZPixmap, 0, (char*)pixels, W, H, 32, 0);

    GC gc = DefaultGC(d, s);
    int running = 1;
    int is_drawing = 0;

    printf("Программа запущена. Нажмите на крестик для проверки...\n");

    while (running) {
        while (XPending(d)) {
            XEvent ev;
            XNextEvent(d, &ev);

            // ПРОВЕРКА КРЕСТИКА (ClientMessage)
            if (ev.type == ClientMessage) {
                // ВАЖНО: сравниваем именно data.l[0]
                if ((Atom)ev.xclient.data.l[0] == wmDeleteMessage) {
                    printf("Событие закрытия получено!\n");
                    running = 0;
                }
            }

            if (ev.type == KeyPress) running = 0;

            if (ev.type == ButtonPress && ev.xbutton.button == 1) is_drawing = 1;
            if (ev.type == ButtonRelease && ev.xbutton.button == 1) is_drawing = 0;

            if (is_drawing && (ev.type == MotionNotify)) {
                int x = ev.xmotion.x;
                int y = ev.xmotion.y;
                if (x >= 1 && x < W-1 && y >= 1 && y < H-1) {
                    for(int dy = 0; dy <= 0; dy++) {
                        for(int dx = 0; dx <= 0; dx++) {
                            pixels[(y + dy) * W + (x + dx)] = 0xFF00FF00;
                        }
                    }
                }
            }
        }

        XPutImage(d, w, gc, image, 0, 0, 0, 0, W, H);
        XFlush(d);
    }

    XDestroyImage(image); 
    XCloseDisplay(d);
    printf("Окно успешно закрыто через API.\n");
    return 0;
}

