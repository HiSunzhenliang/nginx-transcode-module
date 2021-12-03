/**
 * @file ngx_http_transcode_module_utility.h
 * @author sunzhenliang (hisunzhenliang@outlook.com)
 * @brief utilities
 * @date 2021-12-03
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef TRANSCODE_MODULE_UTILITY_H
#define TRANSCODE_MODULE_UTILITY_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/**
 * @brief generate full path of target file
 *
 * @param pool alloc from this mem pool
 * @param log log
 * @param root root directive
 * @param uri uri
 * @return ngx_str_t alloced str
 */
ngx_str_t generate_path(ngx_pool_t *pool, ngx_log_t *log, ngx_str_t root, ngx_str_t uri);

/**
 * @brief match file by namebase, discard extension
 *
 * @param pool mem pool
 * @param log log
 * @param path have needed namebase and dir to search in
 * @return ngx_str_t alloced str
 */
ngx_str_t match_path(ngx_pool_t *pool, ngx_log_t *log, ngx_str_t path);

/**
 * @brief Get the output format object
 *
 * @param pool mem pool
 * @param uri uri
 * @return ngx_str_t
 */
ngx_str_t get_output_format(ngx_pool_t *pool, ngx_str_t uri);

/**
 * @brief Get the namebase object
 *
 * @param pool mem pool
 * @param path path
 * @return ngx_str_t namebase
 */
ngx_str_t get_namebase(ngx_pool_t *pool, ngx_str_t path);

/**
 * @brief Get the dir object
 *
 * @param pool mem pool
 * @param path path
 * @return ngx_str_t dir
 */
ngx_str_t get_dir(ngx_pool_t *pool, ngx_str_t path);

#endif /* TRANSCODE_MODULE_UTILITY_H */