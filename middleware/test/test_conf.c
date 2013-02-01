#include    <stdio.h>
#include    "mls_config.h"
#include    "mls_log.h"

#define TEST_INIFILE  "./test.ini"

#define CHECK(c,k)                          \
    ({                                      \
        char *val;                          \
        val = mls_conf_get((c), (k));       \
        printf("GET: %s = %s\n", (k), (NULL != val) ? val : "NOTFOUND"); \
    })

#define SET(c,k,v)                              \
    ({                                          \
        int ret = 0;                            \
        ret     = mls_conf_set((c), (k), (v));  \
        if (0 != ret) {                         \
            MLS_FATAL_PERROR("Set error %s:%s", (k), (v)); \
        }                                       \
        printf("SET: %s = %s\n", (k), (v));     \
    })

int
main(int ac, char *argv[])
{
    struct mls_conf* conf;
    conf = mls_conf_ini(TEST_INIFILE);
    if (NULL == conf) {
        MLS_FATAL_PERROR("mls_conf_ini");
    }

    mls_conf_load(conf);
    {
        CHECK(conf, "key1");
        CHECK(conf, "key2");
        CHECK(conf, "ckey1");
        CHECK(conf, "undef");
        CHECK(conf, "keyX");
    }

    {
        SET(conf, "key2", "modify-val2");
        SET(conf, "key3", "val3");
        CHECK(conf, "key2");
        CHECK(conf, "key3");
        CHECK(conf, "keyX");
    }
    mls_conf_store(conf);

    mls_conf_fin(conf);
    return 0;
}
