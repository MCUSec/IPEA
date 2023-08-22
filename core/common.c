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