#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

typedef struct {
    ngx_uint_t  enable;
} ngx_http_webp_conf_t;

static char *ngx_http_webp(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t ngx_http_webp_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_webp_commands[] = {

    { ngx_string("webp"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_webp,
      0,
      0,
      NULL},

    ngx_null_command
};

static ngx_http_module_t ngx_http_webp_module_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

ngx_module_t ngx_http_webp_module = {
    NGX_MODULE_V1,
    &ngx_http_webp_module_ctx,
    ngx_http_webp_commands,
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

static ngx_int_t ngx_http_webp_handler(ngx_http_request_t *r)
{
    ngx_buf_t *b;
    ngx_chain_t out;
    u_char    *p;
    u_char *d;
    ngx_str_t  lpath;
    size_t    root;
    int   status;
    pid_t parent_pid;
    pid_t child_pid;
    parent_pid = getpid();
    child_pid = fork();

    ngx_open_file_info_t            of;
    ngx_http_core_loc_conf_t       *clcf;
    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

    ngx_uint_t                      level;
    ngx_log_t                      *log;

    u_char *accept, *pos;

    accept = r->headers_in.accept->value.data;
    pos = ngx_strnstr(accept, (char *)"webp", 50);

    if (pos != NULL) {
       ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "accept webp status: %s\n", accept);
    }

    p = ngx_http_map_uri_to_path(r, &lpath, &root, sizeof(".webp") - 1);

    lpath.len = p - lpath.data;

    ngx_str_t dpath;
    d = ngx_http_map_uri_to_path(r, &dpath, &root, sizeof(".webp") - 1);
    *d++ = '.';
    *d++ = 'w';
    *d++ = 'e';
    *d++ = 'b';
    *d++ = 'p';
    *d = '\0';
    dpath.len = d - dpath.data;

    if (pos == NULL) {
      d = p;
      dpath = lpath;
      dpath.data = lpath.data;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "Source filename status: \"%s\"", lpath.data);
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "Destination filename status: \"%s\"", dpath.data);

    if (pos != NULL) {
    switch( child_pid )
     {
       case 0:
       execlp( "/etc/nginx/webp","/etc/nginx/webp",lpath.data,dpath.data, NULL );
       default:
       break;
     }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "parent pid status: %d\n", parent_pid);
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "child pid status: %d\n", child_pid);

    wait( &status );
    waitpid(child_pid, &status, WEXITED);

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "status: %d\n", status);


    if ( status != 0 ){
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
     }
    }
    if (pos == NULL) {
      d = p;
      dpath = lpath;
      dpath.data = lpath.data;
    }


    log = r->connection->log;

    ngx_memzero(&of, sizeof(ngx_open_file_info_t));
    of.read_ahead = clcf->read_ahead;
    of.directio = clcf->directio;
    of.valid = clcf->open_file_cache_valid;
    of.min_uses = clcf->open_file_cache_min_uses;
    of.errors = clcf->open_file_cache_errors;
    of.events = clcf->open_file_cache_events;

    if (ngx_http_set_disable_symlinks(r, clcf, &dpath, &of) != NGX_OK) {
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (ngx_open_cached_file(clcf->open_file_cache, &dpath, &of, r->pool)
        != NGX_OK)
    {

        switch (of.err) {
        case 0:
            return NGX_HTTP_INTERNAL_SERVER_ERROR;

        case NGX_ENOENT:
        case NGX_ENOTDIR:
        case NGX_ENAMETOOLONG:

            return NGX_DECLINED;

        case NGX_EACCES:
#if (NGX_HAVE_OPENAT)
        case NGX_EMLINK:
        case NGX_ELOOP:
#endif
            level = NGX_LOG_ERR;
        break;
        default:
            level = NGX_LOG_CRIT;
            break;
        }

        ngx_log_error(level, log, of.err, "%s \"%s\" failed", of.failed, dpath.data);

        return NGX_DECLINED;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "uri status: %s\n", r->uri.data);

    u_char *mimejpg, *mimepng, *mimejpeg;

    mimepng = ngx_strcasestrn(r->uri.data, (char *)"png", 3 - 1 );
    mimejpg = ngx_strcasestrn(r->uri.data, (char *)"jpg", 3 - 1 );
    mimejpeg = ngx_strcasestrn(r->uri.data, (char *)"jpeg", 4 - 1 );

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = of.size;
    r->headers_out.last_modified_time = of.mtime;

    if (pos != NULL) {
      ngx_table_elt_t                *h;
      h = ngx_list_push(&r->headers_out.headers);
      h->hash = 1;
      ngx_str_set(&h->key, "Content-Type");
      ngx_str_set(&h->value, "image/webp");
      r->headers_out.content_encoding = h;
    }else{
    if (mimepng != NULL){
      ngx_table_elt_t                *h;
      h = ngx_list_push(&r->headers_out.headers);
      h->hash = 1;
      ngx_str_set(&h->key, "Content-Type");
      ngx_str_set(&h->value, "image/png");
      r->headers_out.content_encoding = h;
    }
    if (mimejpg != NULL || mimejpeg != NULL){
      ngx_table_elt_t                *h;
      h = ngx_list_push(&r->headers_out.headers);
      h->hash = 1;
      ngx_str_set(&h->key, "Content-Type");
      ngx_str_set(&h->value, "image/jpeg");
      r->headers_out.content_encoding = h;
    }
    }

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    b->file = ngx_pcalloc(r->pool, sizeof(ngx_file_t));
    ngx_http_send_header(r);
    b->file_pos = 0;
    b->file_last = of.size;

    b->in_file = b->file_last ? 1 : 0;
    b->last_buf = (r == r->main) ? 1 : 0;
    b->last_in_chain = 1;

    b->file->fd = of.fd;
    out.buf = b;
    out.next = NULL;

    return ngx_http_output_filter(r, &out);
}

static char *ngx_http_webp(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_webp_handler;

    return NGX_CONF_OK;
}
