/*
 * @file ngx_http_transcode_module.h
 * @author SunZhenliang
 */
#ifndef TRANSCODE_MODULE_H
#define TRANSCODE_MODULE_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef enum {
    NGX_HTTP_TRANSCODE_MODULE_FOUND = 0,
    NGX_HTTP_TRANSCODE_MODULE_NOT_FOUND,
    NGX_HTTP_TRANSCODE_MODULE_NO_DECODER,
    NGX_HTTP_TRANSCODE_MODULE_NO_ENCODER,
    NGX_HTTP_TRANSCODE_MODULE_TRANS_ERROR,
    NGX_HTTP_TRANSCODE_MODULE_LIBSOX_ERROR
} status_code;

#endif /* TRANSCODE_MODULE_H */