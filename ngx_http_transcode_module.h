/*
 * @file ngx_http_transcode_module.h
 * @author SunZhenliang
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <sox.h>

typedef enum {
    NGX_HTTP_TRANSCODE_MODULE_FOUND = 0,
    NGX_HTTP_TRANSCODE_MODULE_NOT_FOUND,
    NGX_HTTP_TRANSCODE_MODULE_NO_DECODER,
    NGX_HTTP_TRANSCODE_MODULE_NO_ENCODER,
    NGX_HTTP_TRANSCODE_MODULE_TRANS_ERROR,
    NGX_HTTP_TRANSCODE_MODULE_LIBSOX_ERROR
} status_code;

typedef struct {
    ngx_flag_t enabled;
    ngx_http_complex_value_t *root;
    ngx_http_complex_value_t *output_format;
} ngx_http_transcode_loc_conf_t;

static char *ngx_http_transcode(ngx_conf_t *, ngx_command_t *, void *);

static void *ngx_http_transcode_create_loc_conf(ngx_conf_t *);

static char *ngx_http_transcode_merge_loc_conf(ngx_conf_t *, void *, void *);

static ngx_str_t generate_path(ngx_pool_t *pool, ngx_log_t *log, ngx_str_t root, ngx_str_t uri);

static ngx_str_t match_path(ngx_pool_t *pool, ngx_log_t *log, ngx_str_t path);

static ngx_str_t get_output_format(ngx_pool_t *pool, ngx_str_t uri);

static ngx_int_t transcode(ngx_str_t *output, ngx_pool_t *pool, ngx_log_t *log, ngx_str_t source, ngx_str_t fmt);
