#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

void syscall_init();
extern uint64_t syscall_handler(void);


#endif