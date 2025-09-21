// Copyright (c) 2021 Cesanta Software Limited
// All rights reserved
//
// Example HTTP client. Connect to `s_url`, send request, wait for a response,
// print the response and exit.
// You can change `s_url` from the command line by executing: ./example YOUR_URL
//
// To enable SSL/TLS, , see https://mongoose.ws/tutorials/tls/#how-to-build

#include "mongoose.h"

// #define DISABLE_MAIN 0

// The very first web page in history. You can replace it from command line
static const char *s_url = "http://info.cern.ch/";
static struct mg_str s_ca_pem;             // CA PEM file
static const char *s_post_data = NULL;     // POST data
static const uint64_t s_timeout_ms = 1500; // Connect timeout in milliseconds

// Print HTTP response and signal that we're done
static void fn(struct mg_connection *c, int ev, void *ev_data)
{
    if (ev == MG_EV_OPEN)
    {
        // Connection created. Store connect expiration time in c->data
        *(uint64_t *)c->data = mg_millis() + s_timeout_ms;
    }
    else if (ev == MG_EV_POLL)
    {
        if (mg_millis() > *(uint64_t *)c->data &&
            (c->is_connecting || c->is_resolving))
        {
            mg_error(c, "Connect timeout");
        }
    }
    else if (ev == MG_EV_CONNECT)
    {
        // Connected to server. Extract host name from URL
        struct mg_str host = mg_url_host(s_url);

        if (c->is_tls)
        {
            struct mg_tls_opts opts = {.ca = s_ca_pem, .name = mg_url_host(s_url)};
            mg_tls_init(c, &opts);
        }

        // Send request
        int content_length = s_post_data ? strlen(s_post_data) : 0;
        mg_printf(c,
                  "%s %s HTTP/1.0\r\n"
                  "Host: %.*s\r\n"
                  "Content-Type: octet-stream\r\n"
                  "Content-Length: %d\r\n"
                  "\r\n",
                  s_post_data ? "POST" : "GET", mg_url_uri(s_url), (int)host.len,
                  host.buf, content_length);
        mg_send(c, s_post_data, content_length);
    }
    else if (ev == MG_EV_HTTP_MSG)
    {
        // Response is received. Print it
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        printf("%.*s", (int)hm->message.len, hm->message.buf);
        c->is_draining = 1;         // Tell mongoose to close this connection
        *(bool *)c->fn_data = true; // Tell event loop to stop
    }
    else if (ev == MG_EV_ERROR)
    {
        *(bool *)c->fn_data = true; // Error, tell event loop to stop
    }
}

void http_client_main_thread(int argc, char *argv[])
{
        struct mg_mgr mgr; // Event manager
        bool done = false; // Event handler flips it to true
        int i, log_level = MG_LL_DEBUG;


        mg_mgr_init(&mgr);                       // Initialise event manager
        mg_log_set(log_level);                   // Set log level
        mg_http_connect(&mgr, s_url, fn, &done); // Create client connection
        while (!done)
            mg_mgr_poll(&mgr, 50); // Event manager loops until 'done'
        mg_mgr_free(&mgr);         // Free resources

}
