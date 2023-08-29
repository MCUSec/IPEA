/**
 * @file libipeasan_conf.h
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Configurations of IPEA-San runtime library
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef LIBIPEASAN_CONF_H
#define LIBIPEASAN_CONF_H

#define ASM __attribute__((naked))
#define EXPORT __attribute__((visibility("default")))

#ifndef USAN_ENABLE_CRC
#define USAN_ENABLE_CRC 1
#endif

#ifndef USAN_ENABLE_STACK_PROFILING
#define USAN_ENABLE_STACK_PROFILING 0
#endif

#endif