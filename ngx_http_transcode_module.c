/*
 * @file ngx_http_transcode_module.c
 * @author SunZhenliang
 */

#include "ngx_http_transcode_module.h"

#include <libgen.h>

/* ngx interface */
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
/* ngx interface end */

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
    ngx_str_t matched_path = ngx_null_string;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_transcode_module);
    if (conf->root) {
        ngx_http_complex_value(r, conf->root, &root);
    }
    if (conf->output_format) {
        ngx_http_complex_value(r, conf->output_format, &output_format);
    } else {
        output_format = get_output_format(r->pool, r->uri);
    }

    if (!output_format.data) {
        return NGX_HTTP_BAD_REQUEST;
    }

    err = ngx_http_discard_request_body(r);
    if (err != NGX_OK) {
        return err;
    }

    log = r->connection->log;
    path = generate_path(r->pool, log, root, r->uri);
    if (!path.data) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    matched_path = match_path(r->pool, log, path);
    if (!matched_path.data) {
        return NGX_HTTP_NOT_FOUND;
    }

    code = transcode(&output, r->pool, log, matched_path, output_format);

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
    } else {
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
    ngx_snprintf(path.data, root.len + uri.len, "%V%V", &root, &uri);
    path.len = root.len + uri.len;
    return path;
}

static ngx_str_t get_dir(ngx_pool_t *pool, ngx_str_t path) {
    ngx_str_t dir = ngx_null_string;
    u_char *dirpath = ngx_pcalloc(pool, path.len + 1);
    u_char *p;
    if (!path.data) {
        return dir;
    }
    ngx_snprintf(dirpath, path.len, "%V", &path);
    p = (u_char *)dirname((char *)dirpath);
    if (!p) {
        return dir;
    }
    dir.data = p;
    dir.len = ngx_strlen(p);

    return dir;
}

static ngx_str_t get_namebase(ngx_pool_t *pool, ngx_str_t path) {
    ngx_str_t namebase = ngx_null_string;
    u_char *name = ngx_pcalloc(pool, path.len + 1);
    u_char *p;
    if (!path.data) {
        return namebase;
    }
    ngx_snprintf(name, path.len, "%V", &path);
    p = (u_char *)basename((char *)name);
    if (!p) {
        return namebase;
    }
    u_char *dot = (u_char *)ngx_strchr((char *)p, '.');
    if (dot) {
        *dot = '\0';
    }
    namebase.data = p;
    namebase.len = ngx_strlen(p);

    return namebase;
}

static ngx_str_t match_path(ngx_pool_t *pool, ngx_log_t *log, ngx_str_t path) {
    ngx_str_t matched = ngx_null_string;
    ngx_str_t dirpath = ngx_null_string;
    ngx_str_t target_namebase = ngx_null_string;
    ngx_str_t file_namebase = ngx_null_string;
    ngx_int_t r = 0;
    ngx_dir_t dir;

    dirpath = get_dir(pool, path);
    if (!dirpath.data) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "transcode: get dir fail %V", &path);
        return matched;
    }
    target_namebase = get_namebase(pool, path);
    if (!target_namebase.data) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "transcode: get dir fail %V", &path);
        return matched;
    }
    if (ngx_open_dir(&dirpath, &dir) == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "transcode: cant not open dir %V",
                      &dirpath);
        return matched;
    }

    while (ngx_read_dir(&dir) == NGX_OK) {
        ngx_log_error(NGX_LOG_DEBUG, log, 0, "Traverse {%s,%d}",
                      ngx_de_name(&dir), ngx_de_namelen(&dir));
        file_namebase = get_namebase( pool, (ngx_str_t){ngx_de_namelen(&dir), ngx_de_name(&dir)});
        if (!file_namebase.data ||
            !ngx_strncmp(file_namebase.data, "", file_namebase.len)) {
            continue;
        }

        ngx_log_error(NGX_LOG_DEBUG, log, 0,
                      "file_namebase {%s,%d} target_namebase {%s,%d}",
                      file_namebase.data, file_namebase.len,
                      target_namebase.data, target_namebase.len);

        if (!ngx_filename_cmp(target_namebase.data, file_namebase.data, file_namebase.len)) {
            ngx_log_error(NGX_LOG_DEBUG, log, 0, "Got {%s,%d}", ngx_de_name(&dir), ngx_de_namelen(&dir));
            r = 1;
            break;
        }
    }

    if (r) {
        matched.data = ngx_pcalloc(pool, dirpath.len + ngx_de_namelen(&dir));
        ngx_sprintf(matched.data, "%V/%s", &dirpath, ngx_de_name(&dir));
        matched.len = dirpath.len + ngx_de_namelen(&dir) + 1;
    }

    return matched;
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

    sox_globals.verbosity = 0;
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
    ngx_snprintf((u_char *)output_format, fmt.len, "%V", &fmt);
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

static ngx_str_t get_output_format(ngx_pool_t *pool, ngx_str_t uri) {
    ngx_str_t fmt = ngx_null_string;
    ngx_int_t fmtlen;

    u_char *dot = (u_char *)ngx_strchr(uri.data, '.');
    if (!dot) {
        return fmt;
    }
    fmtlen = uri.data + uri.len - dot - 1;
    fmt.data = ngx_pcalloc(pool, fmtlen * sizeof(u_char));
    ngx_memcpy(fmt.data, dot + 1, fmtlen);
    fmt.len = fmtlen;
    return fmt;
}