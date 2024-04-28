#include "http_client.h"

#include "json/json.h"

#include <stdio.h>
#include <stdlib.h>

using namespace std;

// static const char *s_url = "http://172.29.249.136/api/jrtp";
static string s_url = "http://172.29.249.136/api/jrtp";
static string s_post_data = ""; // POST data
static string s_token = "";     // POST data
dg_response HttpClient::rep;
static const uint64_t s_timeout_ms = 1500; // Connect timeout in milliseconds
std::mutex HttpClient::mtx_connext_;

HttpClient::HttpClient() {
  // const char *log_level = getenv("LOG_LEVEL");  // Allow user to set log
  // level if (log_level == NULL) log_level = "4";       // Default is verbose

  // struct mg_mgr mgr;              // Event manager
  // bool done = false;              // Event handler flips it to true
  // mg_log_set(atoi(log_level));    // Set to 0 to disable debug
  mg_mgr_init(&mgr); // Initialise event manager
}

HttpClient::~HttpClient() { mg_mgr_free(&mgr); }

void HttpClient::fn(struct mg_connection *c, int ev, void *ev_data,
                    void *fn_data) {
  if (ev == MG_EV_OPEN) {
    // Connection created. Store connect expiration time in c->data
    *(uint64_t *)c->data = mg_millis() + s_timeout_ms;
  } else if (ev == MG_EV_POLL) {
    if (mg_millis() > *(uint64_t *)c->data &&
        (c->is_connecting || c->is_resolving)) {
      mg_error(c, "Connect timeout");
    }
  } else if (ev == MG_EV_CONNECT) {
    // Connected to server. Extract host name from URL
    struct mg_str host = mg_url_host(s_url.data());

    // If s_url is https://, tell client connection to use TLS
    // if (mg_url_is_ssl(s_url.data())) {
    //     struct mg_tls_opts opts = {.ca = "ca.pem", .srvname = host};
    //     mg_tls_init(c, &opts);
    // }

    // Send request
    // int content_length = s_post_data.length() > 0 ? strlen(s_post_data) : 0;
    int content_length = s_post_data.length() > 0 ? s_post_data.length() : 0;
    mg_printf(c,
              "%s %s HTTP/1.0\r\n"
              "Host: %.*s\r\n"
              "Authorization:%s"
              "Content-Type: octet-stream\r\n"
              "Content-Length: %d\r\n"
              "\r\n",
              s_post_data.length() > 0 ? "POST" : "GET",
              mg_url_uri(s_url.data()), (int)host.len, host.ptr, s_token.data(),
              content_length);
    mg_send(c, s_post_data.data(), content_length);
  } else if (ev == MG_EV_HTTP_MSG) {
    // Response is received. Print it
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    printf("%.*s", (int)hm->message.len, hm->message.ptr);
    printf("%s", hm->body.ptr);
    rep.body = hm->body.ptr;
    rep.head = hm->head.ptr;
    rep.url = hm->uri.ptr;

    c->is_closing = 1;       // Tell mongoose to close this connection
    *(bool *)fn_data = true; // Tell event loop to stop
  } else if (ev == MG_EV_ERROR) {
    *(bool *)fn_data = true; // Error, tell event loop to stop
  }
}

dg_response HttpClient::request(const std::string &url,
                                const std::string &post_data, string token) {
  // cout << "http client request " << url << endl;
  mtx_connext_.lock();
  bool done = false;
  s_url = url;
  s_post_data = post_data;
  s_token = token;
  memset(&rep, 0, sizeof(rep));
  // mg_http_connect(&mgr, s_url.data(), fn, &done);
  // mg_http_connect(&mgr, "172.29.249.131/api/jrtp", fn, &done);
  mg_http_connect(&mgr, s_url.data(), fn, &done);
  int times = 0;
  while (!done) {
    mg_mgr_poll(&mgr, 50);
    times++;
    if (times > 200)
      break;
  }
  dg_response t_rep = rep;
  mtx_connext_.unlock();
  // cout << rep.body << endl;
  return t_rep;
}
