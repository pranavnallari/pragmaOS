// keyboard.c

#include <drivers/keyboard.h>
#include <drivers/terminal.h>

static const char scancode_map[128] = {
    0,   27,  '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,
    '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' ', 0,
};

void keyboard_handle(uint8_t scancode) {
    if (scancode & 0x80) {
        return;
    }

    const char ch = scancode_map[scancode];
    if (ch != 0) terminal_putchar(global_term, ch);
}