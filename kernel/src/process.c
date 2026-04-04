#include "process.h"
#include "pmm.h"
#include "vga.h"

static process_t processes[MAX_PROCS];
static uint32_t  next_pid   = 0;
static int       proc_count = 0;

void process_init(void) {
    for (int i = 0; i < MAX_PROCS; i++)
        processes[i].state = PROC_DEAD;
}

process_t* process_create(const char* name, void (*entry)(void)) {
    for (int i = 0; i < MAX_PROCS; i++) {
        if (processes[i].state == PROC_DEAD) {
            process_t* p = &processes[i];

            p->pid   = next_pid++;
            p->state = PROC_READY;

            // Copy name
            int j = 0;
            while (name[j] && j < 31) {
                p->name[j] = name[j];
                j++;
            }
            p->name[j] = 0;

            // Zero out CPU state
            for (int k = 0; k < (int)sizeof(cpu_state_t) / 8; k++)
                ((uint64_t*)&p->cpu)[k] = 0;

            // Zero out stack
            for (int k = 0; k < STACK_SIZE / 8; k++)
                p->stack[k] = 0;

            // Stack grows down
            uint64_t stack_top = (uint64_t)p->stack + STACK_SIZE - 8;
            // Align to 16 bytes
            stack_top &= ~0xF;

            p->cpu.rip    = (uint64_t)entry;
            p->cpu.rsp    = stack_top;
            p->cpu.rbp    = stack_top;
            p->cpu.cs     = 0x08;
            p->cpu.ss     = 0x10;
            p->cpu.rflags = 0x202;

            proc_count++;
            return p;
        }
    }
    return 0;
}

process_t* process_get(int index) {
    return &processes[index];
}

int process_count(void) {
    return proc_count;
}