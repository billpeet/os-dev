#include "keyboard.h"
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "../x86.h"
#include "../console.h"

static bool shift;
static bool ctrl;
static bool caps;

char keyboard_get()
{
    u16 status = inb(KEYBOARD_STATUS_PORT);
    if (status & 0x01)
    {

        u8 keycode = inb(KEYBOARD_DATA_PORT);

        if (keycode < 0)
            return 0;

        if (keycode == LEFT_SHIFT_PRESSED || keycode == RIGHT_SHIFT_PRESSED)
            shift = true;
        else if (keycode == LEFT_SHIFT_RELEASED || keycode == RIGHT_SHIFT_RELEASED)
            shift = false;
        else if (keycode == CAPS_LOCK_PRESSED)
            caps = !caps;
        else if (keycode <= 0x39)
        {
            char c = keyboard_map[keycode];
            if (shift || caps)
                c = toupper(c);

            return c;
        }
    }
    return 0;
}

void keyboard_rec()
{
    char c = keyboard_get();
    if (c != 0)
    {
        queue(c);
        wake_interrupt(KEYBOARD_HANDLER_ID);
    }
}