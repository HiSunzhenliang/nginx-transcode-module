/*
 * @file ngx_http_transcode_module.c
 * @author SunZhenliang
 */

#include "ngx_http_transcode_module.h"

static ngx_int_t ngx_http_transcode_handler(ngx_http_request_t *r) {
    ngx_int_t err;
    status_code code;
    ngx_log_t *log;
    ngx_chain_t out;
    ngx_buf_t *buff;
    ngx_http_transcode_loc_conf_t *conf;
    ngx_str_t root = ngx_null_string;
    ngx_str_t output_format = ngx_null_string;
    ngx_str_t output = ngx_null_string;
    ngx_str_t path = ngx_null_string;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_transcode_module);
    ngx_http_complex_value(r, conf->root, &root);
    ngx_http_complex_value(r, conf->output_format, &output_format);

    err = ngx_http_discard_request_body(r);
    if (err != NGX_OK) {
        return err;
    }

    log = r->connection->log;
    path = generate_path(r->pool, root, r->uri);
    code = transcode(&output, r->pool, log, path, output_format);

    switch (code) {
        case NGX_HTTP_TRANSCODE_MODULE_NOT_FOUND:
            r->headers_out.status = NGX_HTTP_NOT_FOUND;
            break;
        case NGX_HTTP_TRANSCODE_MODULE_NO_DECODER:
            r->headers_out.status = NGX_HTTP_NOT_IMPLEMENTED;
            break;
        case NGX_HTTP_TRANSCODE_MODULE_NO_ENCODER:
            r->headers_out.status = NGX_HTTP_NOT_IMPLEMENTED;
            break;
        default:
            r->headers_out.status = NGX_HTTP_OK;
            break;
    }
    if (r->headers_out.status == NGX_HTTP_OK) {
        buff = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
        if (buff == NULL) {
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
        buff->pos = output.data;
        buff->last = output.data + (output.len * sizeof(u_char));
        buff->memory = 1;
        buff->last_buf = 1;
        r->headers_out.content_length_n = output.len;
    }else{
        r->header_only = 1;
        r->headers_out.content_length_n = 0;
    }

    err = ngx_http_send_header(r);
    if (err == NGX_ERROR || err > NGX_OK || r->header_only) {
        return err;
    }

    out.buf = buff;
    out.next = NULL;

    return ngx_http_output_filter(r, &out);
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

static char *ngx_http_transcode_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child) {
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
        ngx_conf_log_error(NGX_LOG_ERR, cf, 0, "transcode: need audio root");
        return NGX_CONF_ERROR;
    }

    if ((!conf->output_format) && (conf->enabled)) {
        ngx_conf_log_error(NGX_LOG_ERR, cf, 0, "transcode: need output format");
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}

static ngx_str_t generate_path(ngx_pool_t *pool, ngx_str_t rootpath, ngx_str_t uri) {
    /* todo */
    ngx_str_t tmp = ngx_null_string;
    return tmp;
}

static ngx_int_t transcode(ngx_str_t *output, ngx_pool_t *pool, ngx_log_t *log, ngx_str_t source, ngx_str_t fmt) {
    /* todo */
    return 0;
}