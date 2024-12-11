#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "config.h"
#include "task.h"

struct process {
    uint16_t id; // process ID
    char filename[PEACHOS_MAX_PATH];
    struct task *task; // the main process task

    // the memory (malloc) allocations of the process
    void *allocations[PEACHOS_MAX_PROGRAM_ALLOCATIONS]; // keep track of all allocations that the user has made to free any non-freed memory at end of run

    // the physical ptr to the process memory
    void *ptr;

    // the physical pointer to the stack memory
    void *stack;

    // the size of the data pointed to by "ptr"
    uint32_t size;

    struct keyboard_buffer {
        char buffer[PEACHOS_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;
};

int process_switch(struct process *process);
int process_load_switch(const char *filename, struct process **process);
int process_load(const char *filename, struct process **process);
int process_load_for_slot(const char *filename, struct process **process, int process_slot);
struct process *process_current();
struct process *process_get(int process_id);

#endif