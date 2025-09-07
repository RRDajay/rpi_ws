// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved
//
// HTTP server example. This server serves both static and dynamic content.
// It opens two ports: plain HTTP on port 8000 and HTTP on port 8443.
// It implements the following endpoints:
//    /api/stats - respond with free-formatted stats on current connections
//    /api/f2/:id - wildcard example, respond with JSON string {"result": "URI"}
//    any other URI serves static files from s_root_dir
//
// To enable SSL/TLS (using self-signed certificates in PEM files),
//    1. See https://mongoose.ws/tutorials/tls/#how-to-build
//    2. curl -k https://127.0.0.1:8443

#include "mongoose.h"
#include <unistd.h>

#define SERVER_MAX_PATH_LEN     1024
#define SERVER_CERT_PATH        "resource/certs/server_cert.pem"
#define SERVER_KEY_PATH         "resource/certs/server_key.pem"
#define CA_CERT_PATH            "resource/certs/ca_cert.pem"

static const char *s_http_addr = "http://0.0.0.0:8000";   // HTTP port
static const char *s_https_addr = "https://0.0.0.0:8443"; // HTTPS port
static const char *s_root_dir = ".";

// We use the same event handler function for HTTP and HTTPS connections
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS
static void fn(struct mg_connection *c, int ev, void *ev_data)
{
    if (ev == MG_EV_ACCEPT && c->is_tls)
    {
        struct mg_tls_opts opts;
        memset(&opts, 0, sizeof(opts));

        char path[SERVER_MAX_PATH_LEN] = {0};
        char s_tls_server_cert_path[SERVER_MAX_PATH_LEN] = {0};
        char s_tls_server_key_path[SERVER_MAX_PATH_LEN] = {0};
        char s_tls_ca_path[SERVER_MAX_PATH_LEN] = {0};

        ssize_t count = readlink("/proc/self/exe", path, SERVER_MAX_PATH_LEN);
        
        if (count != -1) 
        {
            path[count] = '\0';  // Null-terminate the string
        }
        else
        {
            return;
        }

        char *last_slash = strrchr(path, '/');
        if (last_slash != NULL) 
        {
            *last_slash = '\0'; // Terminate at the last '/'
        }
        else
        {
            return;
        }


        snprintf(s_tls_server_cert_path, SERVER_MAX_PATH_LEN, "%s/%s", path, SERVER_CERT_PATH);
        snprintf(s_tls_server_key_path, SERVER_MAX_PATH_LEN, "%s/%s", path, SERVER_KEY_PATH);

#ifdef TLS_TWOWAY
        snprintf(s_tls_ca_path, SERVER_MAX_PATH_LEN, "%s/%s", path, CA_CERT_PATH);
        opts.ca = mg_file_read(&mg_fs_posix, s_tls_ca_path);
#endif
        opts.cert = mg_file_read(&mg_fs_posix, s_tls_server_cert_path);
        opts.key = mg_file_read(&mg_fs_posix, s_tls_server_key_path);
        mg_tls_init(c, &opts);
    }

    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        if (mg_match(hm->uri, mg_str("/api/stats"), NULL))
        {
            struct mg_connection *t;
            // Print some statistics about currently established connections
            mg_printf(c, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            mg_http_printf_chunk(c, "ID PROTO TYPE      LOCAL           REMOTE\n");
            for (t = c->mgr->conns; t != NULL; t = t->next)
            {
                mg_http_printf_chunk(c, "%-3lu %4s %s %M %M\n", t->id,
                                     t->is_udp ? "UDP" : "TCP",
                                     t->is_listening  ? "LISTENING"
                                     : t->is_accepted ? "ACCEPTED "
                                                      : "CONNECTED",
                                     mg_print_ip, &t->loc, mg_print_ip, &t->rem);
            }
            mg_http_printf_chunk(c, ""); // Don't forget the last empty chunk
        }
        else if (mg_match(hm->uri, mg_str("/api/f2/*"), NULL))
        {
            mg_http_reply(c, 200, "", "{\"result\": \"%.*s\"}\n", hm->uri.len,
                          hm->uri.buf);
        }
        else
        {
            struct mg_http_serve_opts opts;
            memset(&opts, 0, sizeof(opts));
            opts.root_dir = s_root_dir;
            mg_http_serve_dir(c, ev_data, &opts);
        }
    }
}

int http_server_main_thread(void)
{
    struct mg_mgr mgr;                            // Event manager
    mg_log_set(MG_LL_DEBUG);                      // Set log level
    mg_mgr_init(&mgr);                            // Initialise event manager
    mg_http_listen(&mgr, s_http_addr, fn, NULL);  // Create HTTP listener
    mg_http_listen(&mgr, s_https_addr, fn, NULL); // HTTPS listener

    while(1)
    {
        mg_mgr_poll(&mgr, 1000); // Infinite event loop
    }

    mg_mgr_free(&mgr);
    return 0;
}