/*
 * logging utility.
 */
#ifndef _MLS_LOG_H_
#define _MLS_LOG_H_

#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

enum mls_log_level {
    MLS_LOG_ALL,
    MLS_LOG_DBG,
    MLS_LOG_INFO,
    MLS_LOG_WARN,
    MLS_LOG_ERR,
    MLS_LOG_FATAL,
    MLS_LOG_NONE
};

#define MLS_LOG_MNAME_LEN 32

struct mls_log_mod {
    int m_mod;
    char m_name[MLS_LOG_MNAME_LEN];
    enum mls_log_level m_level;
};

struct mls_log_conf {
    char* c_path;
    int c_file_size;
    int c_nmod;
    struct mls_log_mod* c_mods;
};

extern struct mls_log_conf* Mls_log_confp;

/*
 * logging functions
 */
extern int mls_log_ini(char* path, int fsize, enum mls_log_level def, int nmod);
extern void mls_log_fin(void);
extern int mls_log_set_module(int mod, char* mod_name, enum mls_log_level);
extern void mls_log_printf(int mod, enum mls_log_level level, const char* func, int line, char* fmt, ...);
extern void mls_log_hexdump(char* data, unsigned long size, FILE*);

static inline int
mls_log_is_output(int mod, enum mls_log_level level)
{
    assert(0 <= mod && mod < Mls_log_confp->c_nmod);
    return (Mls_log_confp->c_mods[mod].m_level <= level);
}

#ifdef NDEBUG
#define LOG_DBG(...) do{}while(0)
#else  /* NDEBUG */
#define LOG_DBG(mod, fmt, ...) do{                              \
    if (mls_log_is_output(mod,MLS_LOG_DBG))                     \
      mls_log_printf(mod, MLS_LOG_DBG, __FUNCTION__, __LINE__,  \
                     fmt, ##__VA_ARGS__);                       \
  }while(0)
#endif /* NDEBUG */

#define LOG_INFO(mod, fmt, ...) do{                             \
    if (mls_log_is_output(mod,MLS_LOG_INFO))                    \
      mls_log_printf(mod, MLS_LOG_INFO, __FUNCTION__, __LINE__, \
                     fmt, ##__VA_ARGS__);                       \
  }while(0)

#define LOG_WARN(mod, fmt, ...) do{                             \
    if (mls_log_is_output(mod,MLS_LOG_WARN))                    \
      mls_log_printf(mod, MLS_LOG_WARN, __FUNCTION__, __LINE__, \
                     fmt, ##__VA_ARGS__);                       \
  }while(0)

#define LOG_ERR(mod, fmt, ...) do{                              \
    if (mls_log_is_output(mod,MLS_LOG_ERR))                     \
      mls_log_printf(mod, MLS_LOG_ERR, __FUNCTION__, __LINE__,  \
                     fmt, ##__VA_ARGS__);                       \
  }while(0)

#define LOG_FATAL(mod, fmt, ...) do{                             \
    if (mls_log_is_output(mod,MLS_LOG_FATAL))                    \
      mls_log_printf(mod, MLS_LOG_FATAL, __FUNCTION__, __LINE__, \
                     fmt, ##__VA_ARGS__);                        \
  }while(0)

/*
 * error handling functions
 */
extern void mls_fatal(char* fmt, ...);
extern void mls_fatal_perror(char* fmt, ...);

#define MLS_FATAL(fmt, ...) do{               \
    mls_fatal(fmt, ##__VA_ARGS__);            \
    abort();              \
  }while(0)

#define MLS_FATAL_PERROR(fmt, ...) do{        \
    mls_fatal_perror(fmt, ##__VA_ARGS__);     \
    abort();              \
  }while(0)

#endif /* _MLS_LOG_H_ */
