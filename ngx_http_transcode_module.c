/*
 * @file ngx_http_transcode_module.c
 * @author SunZhenliang
 */

#include "ngx_http_transcode_module.h"


#define MAX_SAMPLES (size_t)2048
#define MAX_FMT_LEN (size_t)16

static ngx_int_t ngx_http_transcode_handler(ngx_http_request_t *r) {
    ngx_int_t err;
    status_code code;
    ngx_log_t *log;
    ngx_chain_t out;
    ngx_buf_t *buff = NULL;
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
    path = generate_path(r->pool, log, root, r->uri);
    if (!path.data) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

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
        case NGX_HTTP_TRANSCODE_MODULE_LIBSOX_ERROR:
            r->headers_out.status = NGX_HTTP_NOT_IMPLEMENTED;
            break;
        case NGX_HTTP_TRANSCODE_MODULE_TRANS_ERROR:
            r->headers_out.status = NGX_HTTP_INTERNAL_SERVER_ERROR;
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

/*
 * Splice root and URI
 */
static ngx_str_t generate_path(ngx_pool_t *pool, ngx_log_t *log, ngx_str_t root, ngx_str_t uri) {
    ngx_str_t path = ngx_null_string;

    path.data = ngx_pcalloc(pool, (root.len + uri.len) * sizeof(u_char));
    if (!path.data) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "transcode: can not alloc for path");
        ngx_str_null(&path);
        return path;
    }

    path.len = root.len + uri.len;
    ngx_memcpy(path.data, root.data, root.len);
    ngx_memcpy(path.data + root.len, uri.data, uri.len);
    return path;
}

static ngx_int_t transcode(ngx_str_t *output, ngx_pool_t *pool, ngx_log_t *log, ngx_str_t source, ngx_str_t fmt) {
    ngx_int_t code;
    sox_format_t *in = NULL;
    sox_format_t *out = NULL;
    char output_format[MAX_FMT_LEN] = {0};
    char *buffer = NULL;
    size_t buffer_size;
    size_t number_read;
    ngx_int_t open_libsox = 0;
    sox_sample_t samples[MAX_SAMPLES];
    char source_path[NGX_MAX_PATH] = {0};

    ngx_snprintf((u_char *)source_path, source.len, "%V", &source);
    if (access(source_path, F_OK)) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "transcode: input file not found : %s.", source_path);
        code = NGX_HTTP_TRANSCODE_MODULE_NOT_FOUND;
        goto err;
    }

    if (sox_init() != SOX_SUCCESS) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "transcode: libsox init fail.");
        code = NGX_HTTP_TRANSCODE_MODULE_LIBSOX_ERROR;
        goto err;
    }
    open_libsox = 1;

    in = sox_open_read(source_path, NULL, NULL, NULL);
    if (!in) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "transcode: encoder not found.");
        code = NGX_HTTP_TRANSCODE_MODULE_NO_ENCODER;
        goto err;
    }

    if (fmt.len >= MAX_FMT_LEN) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "transcode: format name too long.");
        code = NGX_HTTP_INTERNAL_SERVER_ERROR;
        goto err;
    }
    ngx_snprintf((u_char*)output_format, fmt.len, "%V", &fmt);
    out = sox_open_memstream_write(&buffer, &buffer_size, &in->signal, NULL, output_format, NULL);
    if (!out) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "transcode: decoder not found.");
        code = NGX_HTTP_TRANSCODE_MODULE_NO_DECODER;
        goto err;
    }

    while ((number_read = sox_read(in, samples, MAX_SAMPLES))) {
        sox_write(out, samples, number_read);
    }
    fflush(out->fp);

    sox_close(out);
    sox_close(in);

    output->data = ngx_pcalloc(pool, buffer_size * sizeof(u_char));
    output->len = buffer_size;
    ngx_memcpy(output->data, buffer, buffer_size);

    free(buffer);
    sox_quit();
    return NGX_OK;
err:
    if (in) {
        sox_close(in);
    }
    if (out) {
        sox_close(out);
    }
    if (buffer) {
        free(buffer);
    }
    if (open_libsox) {
        sox_quit();
    }
    return code;
}