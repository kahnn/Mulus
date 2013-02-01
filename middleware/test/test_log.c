#include    <stdio.h>
#include    <string.h>
#include    "mls_log.h"

int
main(int ac, char *argv[])
{
    int ret = 0;

    ret = mls_log_ini("stderr", (1024*1024), MLS_LOG_ALL, 2);
    if (0 != ret) {
        MLS_FATAL_PERROR("mls_log_ini - %d", ret);
    }

    {
        char* data = "fedcba9876543210XYZ";
        unsigned char data2[] = {0x01, 0x02, 0x11, 0x30, 0xfe, 0xfa, 0xa2};
        fprintf(stderr,
            "Address   |>>     8bytes      >>| |>>      8bytes     <<|  |>>  to char  <<|\n");
        mls_log_hexdump(data, strlen(data) + 1, stderr);
        mls_log_hexdump((char *)data2, sizeof(data2), stderr);
    }

    {
        printf("before mls_log_set_module>> \n");
        LOG_DBG(0, "debug print");
        LOG_INFO(0, "information print");
        LOG_WARN(0, "warning print");
        LOG_ERR(0, "error print");
        LOG_FATAL(0, "fatal print");
    }

    {
        ret = mls_log_set_module(0, "NEWMOD-0", MLS_LOG_WARN);
        if (0 != ret) {
            MLS_FATAL("mls_log_set_module - %d", ret);
        }
    }

    {
        printf("after mls_log_set_module>> \n");
        LOG_DBG(0, "debug print");
        LOG_INFO(0, "information print");
        LOG_WARN(0, "warning print");
        LOG_ERR(0, "error print");
        LOG_FATAL(0, "fatal print");
        LOG_DBG(1, "debug print");
        LOG_INFO(1, "information print");
        LOG_WARN(1, "warning print");
        LOG_ERR(1, "error print");
        LOG_FATAL(1, "fatal print");
    }

    mls_log_fin();

    return 0;
}
