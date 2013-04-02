/*
 * logging utility.
 */
/*
 * @note XXXX MT-UN-Safe
 */
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mls_log.h"

////////////////////////////////////////////////////////////////

static struct mls_log_conf _mls_log_conf;
struct mls_log_conf* Mls_log_confp = &_mls_log_conf;

static struct
{
    enum mls_log_level level;
    char *name;
} levels[] = {
    { MLS_LOG_ALL, "ALL"},
    { MLS_LOG_DBG, "DBG"},
    { MLS_LOG_INFO, "INFO"},
    { MLS_LOG_WARN, "WARN"},
    { MLS_LOG_ERR, "ERR"},
    { MLS_LOG_FATAL, "FATAL"},
    { MLS_LOG_NONE, "NONE"},
};

static FILE* _outfp; /* default logging file */

static inline char*
_get_lvname(enum mls_log_level level)
{
    assert(0 <= level && level < sizeof(levels)/sizeof(levels[0]));
    return levels[level].name;
}

static inline char*
_get_modname(int mod)
{
    assert(0 <= mod && mod < _mls_log_conf.c_nmod);
    return _mls_log_conf.c_mods[mod].m_name;
}

static inline FILE*
_open_file(char *path)
{
    /* TODO: XXXX Check path & open file */
    return stderr;
}

static inline void
_close_file(FILE* fp)
{
    /* TODO: XXXXC Close file */
#if 0
    fclose(fp);
#endif
}

int
mls_log_ini(char* path, int fsize, enum mls_log_level def, int nmod)
{
    char *ph = NULL;
    struct mls_log_mod* mods = NULL;
    int ret = 0;
	
    mods = malloc(sizeof(*mods) * nmod);
    ph = strdup(path);
    if (NULL == mods || NULL == ph) {
        ret = -errno;
        goto out;
    }

    _outfp = _open_file(path);

    memset(&_mls_log_conf, 0, sizeof(_mls_log_conf));
    _mls_log_conf.c_path = ph;
    _mls_log_conf.c_file_size = fsize;
    {
        int i;
        char defname[MLS_LOG_MNAME_LEN];
        for (i = 0; i < nmod; i++) {
            snprintf(defname, MLS_LOG_MNAME_LEN, "mod-%d", i);
            mods[i].m_mod = i;
            strncpy(mods[i].m_name, defname, (MLS_LOG_MNAME_LEN - 1));
            mods[i].m_level = def;
        }
    }
    _mls_log_conf.c_nmod = nmod;
    _mls_log_conf.c_mods = mods;
out:
    if (ret) {
        if (NULL != mods) free(mods);
        if (NULL != ph) free(ph);
    }
    return ret;
}

void
mls_log_fin(void)
{
    _close_file(_outfp);
    free(_mls_log_conf.c_path);
    free(_mls_log_conf.c_mods);

    memset(&_mls_log_conf, 0, sizeof(_mls_log_conf));
}

int
mls_log_set_module(int mod, char* mod_name, enum mls_log_level level)
{
    assert(0 <= mod && mod < _mls_log_conf.c_nmod);

    strncpy(_mls_log_conf.c_mods[mod].m_name, mod_name, 
        (MLS_LOG_MNAME_LEN - 1));
    _mls_log_conf.c_mods[mod].m_level = level;

    return 0;
}

void
mls_log_printf(int mod, enum mls_log_level level, const char* func, int line,
    char* fmt, ...)
{
    va_list ap;
    time_t timer;
    struct tm dater;

    time(&timer);
    localtime_r(&timer, &dater);

    fprintf(_outfp, "%4d/%02d/%02dT%02d:%02d:%02d %s:%s:%s@%d: ", 
        (dater.tm_year + 1900), (dater.tm_mon + 1), dater.tm_mday, 
        dater.tm_hour, dater.tm_min, dater.tm_sec,
        _get_modname(mod), _get_lvname(level), func, line);
    va_start(ap, fmt);
    vfprintf(_outfp, fmt, ap);
#if 0
    fprintf(_outfp, "\n");
#endif
}

///////////////////////////////////////////////////////////////

void
mls_fatal(char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

void
mls_fatal_perror(char* fmt, ...)
{
    int error = errno;
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, ": %s\n", strerror(error));
}

///////////////////////////////////////////////////////////////

/*
Address   |>>     8bytes      >>| |>>      8bytes     <<|  |>>  to char  <<|
0----0---90----1---90----2---90----3---90----4---90----5---90----6---9012345
12345678  00 11 22 33 44 55 66 77-88 99 AA BB CC DD EE FF  01234567890abcdef
*/
void
mls_log_hexdump(char* data, unsigned long size, FILE* outfp)
{
    unsigned long addr = 0;

    if (NULL == outfp)
        outfp = _outfp;

    while (addr < size) {
        char lbuf[80+1], *lpos = lbuf;
        char cbuf[16+1];
        int cpos;
		
        lpos += sprintf(lpos, "  %08lX  ", addr);

        for (cpos = 0; cpos < 16; cpos++) {
            char ch, *dl;
            if (size <= addr) {
                cbuf[cpos] = '\0';
                for (;cpos < 16; cpos++) {
                    lpos += sprintf(lpos, "   ");
                }
                goto output;
            }
            ch = data[addr];
            lpos += sprintf(lpos, "%02X", ((unsigned char)ch & 0xff));
            dl = (7 == cpos) ? "  " : " ";
            lpos += sprintf(lpos, "%s", dl);

            cbuf[cpos] = (0x20 <= ch && ch < 0x7F) ? ch : '.';
            addr++;
        }
        cbuf[cpos] = '\0';
  output:
        sprintf(lpos, "  %s", cbuf);
        fprintf(outfp, "%s\n", lbuf);
    }
}
