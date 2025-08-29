#pragma once
#define DETAIL_PRINT 
#include <stdio.h>
#include <stdlib.h>

#define CHECK_RETURN_0(ptr) \
    if (!(ptr)) return 0;

#define CHECK_MSG_RETURN_0(ptr, msg) \
    if (!(ptr)) { printf("%s\n", msg); return 0; }

#define FREE_CLOSE_FILE_RETURN_0(ptr, fp) \
    if ((ptr)) free(ptr); \
    if ((fp)) fclose(fp); \
    return 0;

#define CLOSE_RETURN_0(fp) \
    if ((fp)) fclose(fp); \
    return 0;

#define CHECK_MSG_CLOSE_RETURN_0(ptr, msg, fp) \
    if (!(ptr)) { printf("%s\n", msg); fclose(fp); return 0; }
