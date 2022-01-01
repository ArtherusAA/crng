/* Simple command-line kernel monitor useful for
 * controlling the kernel and exploring the system interactively. */

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/env.h>
#include <inc/x86.h>
#include <inc/crng.h>
#include <inc/nist.h>
#include <inc/ecdsa.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/tsc.h>
#include <kern/timer.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/kclock.h>

#define WHITESPACE "\t\r\n "
#define MAXARGS    16

extern curve p_192;

/* Functions implementing monitor commands */
int mon_help(int argc, char **argv, struct Trapframe *tf);
int mon_kerninfo(int argc, char **argv, struct Trapframe *tf);
int mon_backtrace(int argc, char **argv, struct Trapframe *tf);
int mon_dumpcmos(int argc, char **argv, struct Trapframe *tf);
int mon_start(int argc, char **argv, struct Trapframe *tf);
int mon_stop(int argc, char **argv, struct Trapframe *tf);
int mon_frequency(int argc, char **argv, struct Trapframe *tf);
int mon_memory(int argc, char **argv, struct Trapframe *tf);
int mon_pagetable(int argc, char **argv, struct Trapframe *tf);
int mon_virt(int argc, char **argv, struct Trapframe *tf);
int mon_crng(int argc, char **argv, struct Trapframe *tf);
int mon_crng_doom(int argc, char **argv, struct Trapframe *tf);
int mon_crng_test(int argc, char **argv, struct Trapframe *tf);
int mon_ecdsa_test(int argc, char **argv, struct Trapframe *tf);

struct Command {
    const char *name;
    const char *desc;
    /* return -1 to force monitor to exit */
    int (*func)(int argc, char **argv, struct Trapframe *tf);
};

static struct Command commands[] = {
        {"help", "Display this list of commands", mon_help},
        {"kerninfo", "Display information about the kernel", mon_kerninfo},
        {"backtrace", "Print stack backtrace", mon_backtrace},
        {"dumpcmos", "Print CMOS contents", mon_dumpcmos},
        {"timer_start", "Run timer_start", mon_start},
        {"timer_stop", "Run timer_stop", mon_stop},
        {"timer_freq", "Run timer_cpu_frequency", mon_frequency},
        {"memory", "Dump memory lists", mon_memory},
        {"virtual_memory", "Dump virtual tree", mon_virt},
        {"page_table", "Dump page table", mon_pagetable},
        {"crng", "Print random unsinged integer", mon_crng},
        {"crng_doom", "Print pseudo-random unsinged integer", mon_crng_doom},
        {"crng_test", "Test crng", mon_crng_test},
        {"ecdsa_test", "Test ecdsa", mon_ecdsa_test}
};
#define NCOMMANDS (sizeof(commands) / sizeof(commands[0]))

/* Implementations of basic kernel monitor commands */

int
mon_help(int argc, char **argv, struct Trapframe *tf) {
    for (size_t i = 0; i < NCOMMANDS; i++)
        cprintf("%s - %s\n", commands[i].name, commands[i].desc);
    return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf) {
    extern char _head64[], entry[], etext[], edata[], end[];

    cprintf("Special kernel symbols:\n");
    cprintf("  _head64 %16lx (virt)  %16lx (phys)\n", (unsigned long)_head64, (unsigned long)_head64);
    cprintf("  entry   %16lx (virt)  %16lx (phys)\n", (unsigned long)entry, (unsigned long)entry - KERN_BASE_ADDR);
    cprintf("  etext   %16lx (virt)  %16lx (phys)\n", (unsigned long)etext, (unsigned long)etext - KERN_BASE_ADDR);
    cprintf("  edata   %16lx (virt)  %16lx (phys)\n", (unsigned long)edata, (unsigned long)edata - KERN_BASE_ADDR);
    cprintf("  end     %16lx (virt)  %16lx (phys)\n", (unsigned long)end, (unsigned long)end - KERN_BASE_ADDR);
    cprintf("Kernel executable memory footprint: %luKB\n", (unsigned long)ROUNDUP(end - entry, 1024) / 1024);
    return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf) {
    // LAB 2: Your code here
    cprintf("Stack backtrace:\n");
    //uint64_t test;
    //cprintf("test %016lx\n", (long unsigned int)(&test+3));
    struct Ripdebuginfo info;
    uint64_t *rbp = (uint64_t *)read_rbp();
    uint64_t rip;
    while (rbp) {
        rip = *(rbp + 1);
        debuginfo_rip(rip, &info);
        cprintf("  rbp %016lx rip %016lx\n", (long unsigned int)rbp, (long unsigned int)rip);
        cprintf("    %.s:%d: %.*s+%ld\n", info.rip_file,
                                          info.rip_line,
                                          info.rip_fn_namelen,
                                          info.rip_fn_name,
                                          (rip - info.rip_fn_addr));
        rbp = (uint64_t *)(*rbp);
    };
    return 0;
}

int
mon_dumpcmos(int argc, char **argv, struct Trapframe *tf) {
    // Dump CMOS memory in the following format:
    // 00: 00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
    // 10: 00 ..
    // Make sure you understand the values read.
    // Hint: Use cmos_read8()/cmos_write8() functions.
    // LAB 4: Your code here
    size_t offset = 0x0;
    for (size_t i = CMOS_START; i < CMOS_SIZE + CMOS_START; ++i) {
        if (offset % 0x10 == 0) {
            if (offset) cprintf("\n");
            cprintf("%02lx: ", offset);
        }
        cprintf("%02x ", cmos_read8(i));
        offset++;
    }
    cprintf("\n");
    return 0;
}

/* Implement timer_start (mon_start), timer_stop (mon_stop), timer_freq (mon_frequency) commands. */
// LAB 5: Your code here:
int mon_start(int argc, char **argv, struct Trapframe *tf) {
    if (argc != 2) {
        return 1;
    }
    timer_start(argv[1]);
    return 0;
}

int mon_stop(int argc, char **argv, struct Trapframe *tf) {
    timer_stop();
    return 0;
}

int mon_frequency(int argc, char **argv, struct Trapframe *tf) {
    if (argc != 2) {
        return 1;
    }
    timer_cpu_frequency(argv[1]);
    return 0;
}

int mon_crng_test(int argc, char **argv, struct Trapframe *tf) {
    uint64_t (*tested_function)() = secure_urand64_rdrand;
    int tests_size = 200, count = 0;
    unsigned n = 750000, M = 10000;
    if (argc < 2) {
        cprintf("No tested function specified, leave by default: secure_urand64_rdrand\n");
        cprintf("Testing CPRNG with hardware entropy(n size: %u, M size: %u):\n", n, M);
    } else if (!strcmp(argv[1], "rdrand")) {
        tested_function = secure_urand64_rdrand;
        cprintf("Testing CPRNG with hardware entropy(n size: %u, M size: %u):\n", n, M);
    } else if (!strcmp(argv[1], "doom")) {
        tested_function = secure_urand64_doom;
        cprintf("Testing CPRNG with fake entropy(n size: %u, M size: %u):\n", n, M);
    } else {
        cprintf("Unknown function, terminate testing\n");
        return 1;
    }
    bool (*test_function[5])(unsigned, unsigned, uint64_t (*)()) = {
        frequency_test,
        frequency_block_test,
        runs_test,
        longest_run_of_ones_test,
        binary_matrix_rank_test
    };
    const char *test_function_name[5] = {
        "Frequency test",
        "Frequency block test",
        "Runs test",
        "Longest run of ones test",
        "Binary matrix rank test"
    };
    for (int test_count = 0; test_count < sizeof(test_function) / sizeof(test_function[0]); test_count++) {
        cprintf("-%s:\n--Testing", test_function_name[test_count]);
        for (int i = 1; i <= tests_size; i++) {
            if (test_function[test_count](n, M, tested_function)) {
                count += 1;
            }
            if (i % (tests_size / 10) == 0) {
                cprintf(".");
            }
        }
        cprintf("OK\n--Result: %d/%d tests passed\n", count, tests_size);
        count = 0;
    }
    return 0;
}

int
mon_ecdsa_test(int argc, char **argv, struct Trapframe *tf) {
    bignum z; //hash
    char hash[HASHSIZE];
    if (argc < 2) {
        cprintf("no string\n");
        return 1;
    }
    static unsigned char modified_message[1024];
    char *message = argv[1];
    int len = strlen(message);
    md5(message, len, hash);
    convert_from_md5_to_bignum(&z, hash);

    point HA;
    bignum k, s, r, dA;
    bignum_from_int(&k, 7);
    bignum_from_int(&dA, 11);

    ecdsa_sign(&p_192, &z, &dA, &r, &s);
    ecdsa_public_key(&dA, &p_192, &HA);

    int result = ecdsa_verify(&z, &r, &s, &p_192, &HA);

    cprintf("Check signature test: ");
    if (result == 1)
    {
        cprintf("OK\n");
    }
    else {
        cprintf("not OK\n");
    }

    point modified_HA;
    bignum_copy(&modified_HA.x, &HA.x);
    bignum_copy(&modified_HA.y, &HA.y);
    modified_HA.zero_flag = 0;
    bignum_inc(&modified_HA.x);

    result = ecdsa_verify(&z, &r, &s, &p_192, &modified_HA);

    cprintf("Check fake public key test: ");
    if (result == 0)
    {
        cprintf("OK\n");
    }
    else {
        cprintf("not OK\n");
    }

    strncpy((char*)modified_message, message, len);
    modified_message[0] ^= 1U;
    md5((char*)modified_message, len, hash);
    convert_from_md5_to_bignum(&z, hash);

    result = ecdsa_verify(&z, &r, &s, &p_192, &modified_HA);

    cprintf("Check wrong message test: ");
    if (result == 0)
    {
        cprintf("OK\n");
    }
    else {
        cprintf("not OK\n");
    }
    return 0;
}

/* Implement memory (mon_memory) command.
 * This command should call dump_memory_lists()
 */
// LAB 6: Your code here
int
mon_memory(int argc, char **argv, struct Trapframe *tf) {
    dump_memory_lists();
    return 0;
}

/* Implement mon_pagetable() and mon_virt()
 * (using dump_virtual_tree(), dump_page_table())*/
// LAB 7: Your code here
int
mon_virt(int argc, char **argv, struct Trapframe *tf) {
    dump_virtual_tree(&root, 0);
    return 0;
}
int
mon_pagetable(int argc, char **argv, struct Trapframe *tf) {
    dump_page_table(kspace.pml4);
    return 0;
}

int
mon_crng(int argc, char **argv, struct Trapframe *tf) {
    cprintf("%lu\n", secure_urand64_rdrand());
    return 0;
}

int
mon_crng_doom(int argc, char **argv, struct Trapframe *tf) {
    cprintf("%lu\n", secure_urand64_doom());
    return 0;
}

/* Kernel monitor command interpreter */

static int
runcmd(char *buf, struct Trapframe *tf) {
    int argc = 0;
    char *argv[MAXARGS];

    argv[0] = NULL;

    /* Parse the command buffer into whitespace-separated arguments */
    for (;;) {
        /* gobble whitespace */
        while (*buf && strchr(WHITESPACE, *buf)) *buf++ = 0;
        if (!*buf) break;

        /* save and scan past next arg */
        if (argc == MAXARGS - 1) {
            cprintf("Too many arguments (max %d)\n", MAXARGS);
            return 0;
        }
        argv[argc++] = buf;
        while (*buf && !strchr(WHITESPACE, *buf)) buf++;
    }
    argv[argc] = NULL;

    /* Lookup and invoke the command */
    if (!argc) return 0;
    for (size_t i = 0; i < NCOMMANDS; i++) {
        if (strcmp(argv[0], commands[i].name) == 0)
            return commands[i].func(argc, argv, tf);
    }

    cprintf("Unknown command '%s'\n", argv[0]);
    return 0;
}

void
monitor(struct Trapframe *tf) {

    cprintf("Welcome to the JOS kernel monitor!\n");
    cprintf("Type 'help' for a list of commands.\n");

    if (tf) print_trapframe(tf);
    char *buf;
    do buf = readline("K> ");
    while (!buf || runcmd(buf, tf) >= 0);
}
