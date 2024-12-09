#include "keyboard.h"
#include "status.h"
#include "kernel.h"
#include "task/process.h"
#include "task/task.h"

static struct keyboard *keyboard_list_head = 0x0;
static struct keyboard *keyboard_list_last = 0x0;

void keyboard_init() {

}

int keyboard_inserT(struct keyboard *keyboard) {
    int res = 0x0;
    if (keyboard->init == 0x0) {
        res = EINVARG;
        goto out;
    }

    // keyboard already inserted before
    if (keyboard_list_last) {
        keyboard_list_last->next = keyboard;
        keyboard_list_last = keyboard;
    } else {
        keyboard_list_head = keyboard;
        keyboard_list_last = keyboard;
    }
    res = keyboard->init();
out:
    return res;
}

static int keyboard_get_tail_index(struct process *process) {
    return process->keyboard.tail % sizeof(process->keyboard.buffer);
}

void keyboard_backspace(struct process *process) {
    process->keyboard.tail -= 1;
    int real_index = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index] = 0x00;
}

void keyboard_push(char c) {
    struct process *process = process_current();
    if (!process) {
        return;
    }

    int real_index = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index] = c;
    process->keyboard.tail++;
}

char keyboard_pop() {
    if (!task_current()) {
        return 0x0;
    }

    struct process *process = task_current()->process;
    int real_index = process->keyboard.head % sizeof(process->keyboard.buffer);
    char c = process->keyboard.buffer[real_index];
    if (c == '\0') {
        return 0x0; // nothing to pop
    }

    process->keyboard.buffer[real_index] = 0x0;
    process->keyboard.head++;
    return c;
}