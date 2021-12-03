/**
 * @file ngx_http_transcode_module_transcode.h
 * @author sunzhenliang (hisunzhenliang@outlook.com)
 * @brief transcode
 * @date 2021-12-03
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef TRANSCODE_MODULE_TRANSCODE_H
#define TRANSCODE_MODULE_TRANSCODE_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#define MAX_SAMPLES (size_t)2048
#define MAX_FMT_LEN (size_t)16

/**
 * @brief transcode flow
 *
 * @param output transcoded data
 * @param pool mem pool
 * @param log log
 * @param source source data
 * @param fmt target format
 * @return ngx_int_t 0 for succ; others for err type
 */
ngx_int_t transcode(ngx_str_t *output, ngx_pool_t *pool, ngx_log_t *log, ngx_str_t source, ngx_str_t fmt);

#endif /* TRANSCODE_MODULE_TRANSCODE_H */