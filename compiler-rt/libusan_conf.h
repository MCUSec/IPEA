#ifndef LIBUSAN_CONF_H
#define LIBUSAN_CONF_H

#define ASM __attribute__((naked))
#define EXPORT __attribute__((visibility("default")))

#ifndef USAN_ENABLE_CRC
#define USAN_ENABLE_CRC 1
#endif

#ifndef USAN_ENABLE_STACK_PROFILING
#define USAN_ENABLE_STACK_PROFILING 0
#endif

#endif