/**
 * @file ngx_http_transcode_module_utility.c
 * @author sunzhenliang (hisunzhenliang@outlook.com)
 * @brief implements of utilities
 * @date 2021-12-03
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "ngx_http_transcode_module_utility.h"

#include <libgen.h>

ngx_str_t generate_path(ngx_pool_t *pool, ngx_log_t *log, ngx_str_t root, ngx_str_t uri) {
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

ngx_str_t match_path(ngx_pool_t *pool, ngx_log_t *log, ngx_str_t path) {
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

ngx_str_t get_output_format(ngx_pool_t *pool, ngx_str_t uri) {
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

ngx_str_t get_dir(ngx_pool_t *pool, ngx_str_t path) {
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

ngx_str_t get_namebase(ngx_pool_t *pool, ngx_str_t path) {
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