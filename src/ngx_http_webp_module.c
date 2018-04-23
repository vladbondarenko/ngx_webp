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
    ngx_str_t  lpath;
    size_t    root;
    int   status;
    pid_t parent_pid;
    pid_t child_pid;
    parent_pid = getpid();
    child_pid = fork();

    p = ngx_http_map_uri_to_path(r, &lpath, &root, sizeof(".webp") - 1);

    lpath.len = p - lpath.data;

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "http2 filename status: \"%s\"", lpath.data);

    switch( child_pid )
     {
       case 0:
       execlp( "/usr/bin/cwebp", "-q", "70", lpath.data , "-o", "/tmp/webp.webp", NULL );
       default:
       break;
     }
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "parent pid status: %d\n", parent_pid);
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "child pid status: %d\n", child_pid);
    wait( &status );
    waitpid(child_pid, &status, WEXITED);
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "na webp status: %d\n", status);
    if ( status != 0 ){
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    r->headers_out.content_type.len = sizeof("image/webp") - 1;
    r->headers_out.content_type.data = (u_char *) "image/webp";

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    out.buf = b;
    out.next = NULL; /* just one buffer */

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "uri webp status: %s\n", r->uri.data);

    int fd = open ("/tmp/webp.webp", O_RDONLY);
    int size = lseek(fd, 0, SEEK_END);

    b->file = ngx_pcalloc(r->pool, sizeof(ngx_file_t));
    b->file_pos = 0;
    b->file_last = size;

    b->in_file = b->file_last ? 1 : 0;
    b->last_buf = (r == r->main) ? 1 : 0;
    b->last_in_chain = 1;

    b->file->fd = fd;
//    b->file->name = path;
    out.buf = b;
    out.next = NULL;

    r->headers_out.status = NGX_HTTP_OK;

    r->headers_out.content_length_n = size;

    ngx_http_send_header(r);
    unlink("/tmp/webp.webp");
    return ngx_http_output_filter(r, &out);
}

static char *ngx_http_webp(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_webp_handler;

    return NGX_CONF_OK;
}
