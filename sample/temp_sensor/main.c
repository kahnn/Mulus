/*
  温度センサー サンプル実装：
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mls_obj.h"
#include "mls_node.h"
#include "mls_el.h"

/*
 * プロファイル、デバイスとして別ファイルに用意しておいたオブジェクト
 * を取得する関数。実装はオブジェクト定義ファイル内で用意している。
 */
extern struct mls_eoj* mls_el_get_profile(void);
extern struct mls_eoj* mls_el_get_temperature_sensor(void);

#define errlog(fmt, ...) do{                   \
        fprintf(stderr, fmt, ##__VA_ARGS__);      \
  }while(0)
#define showlog(fmt, ...) do{                   \
        fprintf(stdout, fmt, ##__VA_ARGS__);      \
  }while(0)

/**********************************************************************/

/*
 * time interval handler.
 * プログラム全体で定期的に実行したい処理がある場合に登録する。
 * 必要がなければ、登録しなくてもかまわない。
 * デバイス固有の定期処理については、mls_eoj 毎にハンドラが登録可能なので、
 * そちらを利用する。
 */
static void
_timeinterval_handler(struct mls_evt* evt, void* tag)
{
    /* nothing todo. */
#if 0
    struct mls_el_ctx *ctx = (struct mls_el_ctx *)tag;
    showlog("TIMEOUT: %u %s\n",
        (unsigned int)time(NULL), (char*)tag);
#endif
}

/*
 初期化：
   initialize 
   create profile
   create device object
   create node
   initialize network 
   create context
 */
static int
init(char *ifname, struct mls_el_ctx **ctxpp)
{
    int ret = 0;
    struct mls_eoj *profile, *temp_sensor;
    struct mls_node *node;
    struct mls_elnet *elnet;
    struct mls_el_ctx *context;

    /* 必要に応じて独自の情報を渡せるようにする.
       後からは mls_el_get_tag() で取り出せる */
    void *tag = "TemperatureSensor-TAG";

    /* 全体の初期化 */
    mls_el_ini();

    /* Get Objects */
    profile = mls_el_get_profile(); 
    temp_sensor = mls_el_get_temperature_sensor();
    if (NULL == profile || NULL == temp_sensor) {
        ret = -ENOENT;
        goto out;
    }

    /* Create node */
    node = mls_el_node_create(profile);
    if (NULL == node) {
        ret = -ENOENT;
        goto out;
    }
    mls_el_node_add_device(node, temp_sensor);

    /* Initialize network */
    elnet = mls_elnet_init(ifname);
    if (NULL == elnet) {
        ret = -ENOENT;
        goto out;
    }

    /* Initialize EL Context. */
    context = 
        mls_el_create_context(node, elnet, _timeinterval_handler, tag);
    if (NULL == context) {
        errlog("ERROR: mls_el_create_context()\n");
        goto out;
    }
    *ctxpp = context;

out:
    return ret;
}

static void
term(struct mls_el_ctx *ctx)
{
    mls_el_destroy_context(ctx);
    mls_el_fin();
}

/*
 メイン処理：
   announce profile
   event dispatch
 */
static void
run(struct mls_el_ctx *ctx)
{
    mls_el_announce_profile(ctx);
    mls_el_run_context(ctx);
}

/**********************************************************************/
int
main(int argc, char* argv[])
{
    int ret = 0;
    char* ifname = "eth0";
    struct mls_el_ctx *ctx = NULL;

    if (2 <= argc) {
        ifname = argv[1];
    }

    ret = init(ifname, &ctx);
    if (ret < 0) {
        goto out;
    }

    run(ctx);

out:
    term(ctx);
    return ret;
}
