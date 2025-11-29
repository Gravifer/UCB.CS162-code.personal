#include <syscall.h>
#include <stdint.h>

int main(int, char*[]);
void _start(void);

/* Kernel jumps to _start with the initial user stack already prepared.
   Read argc and argv from the stack via %esp directly (no reliance on ebp frame). */
void _start(void) {
    uintptr_t sp;
    /* Get current stack pointer into sp */
    asm volatile("movl %%esp, %0" : "=r"(sp));

    /* At entry the stack layout is:
         sp[0] = argc (int)
         sp[1] = argv[0] (char*)
         sp[2] = argv[1]
         ...
       So:
    */
    int argc = *(int *)sp;
    char **argv = (char **)(sp + sizeof(int));

    exit(main(argc, argv));
}
