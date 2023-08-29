/**
 * @file common.c
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Callbacks for J-Link module
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "device_config.h"

void _LogOut(const char* sLog){
    #if DEBUG_MODE
        printf("Log: %s\n", sLog);
    #endif
}
void _ErrorOut(const char* sError){
    fprintf(stderr, "\nERROR: %s\n", sError);
    exit(-1);
}