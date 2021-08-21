/*
 * @file ngx_http_transcode_module.c
 * @author SunZhenliang
 */

#include "ngx_http_transcode_module.h"

static ngx_int_t ngx_http_transcode_handler(ngx_http_request_t *r) {
    return 1;
}

static char *ngx_http_transcode(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_core_loc_conf_t *clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    ngx_http_transcode_loc_conf_t *tlcf = conf;
    clcf->handler = ngx_http_transcode_handler;
    tlcf->enabled = 1;
    return NGX_CONF_OK;
}

static void *ngx_http_transcode_create_loc_conf(ngx_conf_t *cf) {
    ngx_http_transcode_loc_conf_t *conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_transcode_loc_conf_t));
    if (!conf) {
        return NGX_CONF_ERROR;
    }
    conf->enabled = NGX_CONF_UNSET;
    conf->root = NULL;
    conf->output_format = NULL;

    return conf;
}

static char *ngx_http_transcode_merge_loc_conf(ngx_conf_t *cf, void *parent,
                                        void *child) {
    ngx_http_transcode_loc_conf_t *prev = parent;
    ngx_http_transcode_loc_conf_t *conf = child;

    ngx_conf_merge_value(conf->enabled, prev->enabled, 0);

    if (!conf->root) {
        conf->root = (ngx_http_complex_value_t *)prev->root;
    }
    if (!conf->output_format) {
        conf->output_format = (ngx_http_complex_value_t *)prev->output_format;
    }

    if ((!conf->root) && (conf->enabled)) {
        ngx_conf_log_error( NGX_LOG_ERR, cf, 0, "transcode: need audio root");
        return NGX_CONF_ERROR;
    }

    if ((!conf->output_format) && (conf->enabled)) {
        ngx_conf_log_error(NGX_LOG_ERR, cf, 0, "transcode: need output format");
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}