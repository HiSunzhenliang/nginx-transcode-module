/**
 * @file ngx_http_transcode_module_transcode.c
 * @author sunzhenliang (hisunzhenliang@outlook.com)
 * @brief implements of transcode
 * @date 2021-12-03
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "ngx_http_transcode_module.h"
#include "ngx_http_transcode_module_transcode.h"

#include <sox.h>

ngx_int_t transcode(ngx_str_t *output, ngx_pool_t *pool, ngx_log_t *log, ngx_str_t source, ngx_str_t fmt) {
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