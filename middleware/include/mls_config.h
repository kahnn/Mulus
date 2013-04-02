/*
 * property configuration.
 */
#ifndef _MULUS_CONFIG_H_
#define _MULUS_CONFIG_H_

#define MLS_CONF_LEN_LINE 256
#define MLS_CONF_COMMENT_PREFIX '#'
#define MLS_CONF_KEYVAL_SEP '='

struct mls_conf_prop;
struct mls_conf {
    char *path;
    char *work_path;
    struct mls_conf_prop *root;
};

struct mls_conf_itr {
    struct mls_conf* conf;
    struct mls_conf_prop* next;
};

extern struct mls_conf* mls_conf_ini(char* path);
extern void mls_conf_fin(struct mls_conf*);
extern void mls_conf_load(struct mls_conf*);
extern void mls_conf_store(struct mls_conf*);
extern char* mls_conf_get(struct mls_conf*, char* key);
extern int mls_conf_set(struct mls_conf*, char* key, char* val);
extern int mls_conf_del(struct mls_conf* conf, char* key);
extern void mls_conf_iterator(struct mls_conf* conf, struct mls_conf_itr* itr);
extern int mls_conf_iterator_next(struct mls_conf_itr* itr, char** key, char** val);

#endif /* _MULUS_CONFIG_H_ */
