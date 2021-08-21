/*
 * @file ngx_http_transcode_module.h
 * @author SunZhenliang
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_flag_t enabled;
    ngx_http_complex_value_t *root;
    ngx_http_complex_value_t *output_format;
} ngx_http_transcode_loc_conf_t;

static char *ngx_http_transcode(ngx_conf_t *, ngx_command_t *, void *);

static void *ngx_http_transcode_create_loc_conf(ngx_conf_t *);

static char *ngx_http_transcode_merge_loc_conf(ngx_conf_t *, void *, void *);

static ngx_command_t ngx_http_transcode_commands[] = {
    {
        ngx_string("nginx-transcode-module"),
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
        ngx_http_transcode,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    {
        ngx_string("transcode_root"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_set_complex_value_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_transcode_loc_conf_t, root),
        NULL
    },
    {
        ngx_string("transcode_output_format"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_set_complex_value_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_transcode_loc_conf_t, output_format),
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t ngx_http_transcode_module_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_http_transcode_create_loc_conf,
    ngx_http_transcode_merge_loc_conf
};

ngx_module_t ngx_http_transcode_module = {
    NGX_MODULE_V1,
    &ngx_http_transcode_module_ctx,
    ngx_http_transcode_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};