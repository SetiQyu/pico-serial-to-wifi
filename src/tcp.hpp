/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/cyw43_arch.h"
#include <stdlib.h>

#include "log_buf.hpp"
#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
constexpr int kTcpPort = 4242;
constexpr int kBufSize = 2048;
constexpr int kTestIterations = 10;
constexpr int kPollTime = 1;

#define TESTING 1

#ifdef TESTING
#define DEBUG_printf(...) printf(__VA_ARGS__)
#else
#define DEBUG_printf(...) ((void)0)
#endif

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool complete;
    uint8_t buffer_sent[kBufSize];
    uint8_t buffer_recv[kBufSize];
    int sent_len;
    int recv_len;
    int run_count;
} TCP_SERVER_T;

// The class here has a lot of awkward code because lwip forces one into
// specific type signatures in the functions it wants

class Tcp {
  private:
    inline static int tcpPort = kTcpPort;
    inline static LogBuf *log;
    void *arg;

    static inline TCP_SERVER_T* stateFromArg(void *arg) {
        return static_cast<TCP_SERVER_T*>(arg);
    }

    static err_t tcpAccept(void *arg, struct tcp_pcb *client_pcb, err_t err) {

        TCP_SERVER_T *state = stateFromArg(arg);
        if (err != ERR_OK || client_pcb == NULL) {
            DEBUG_printf("Failure in accept\n");
            // serverResult(arg, err);
            return ERR_VAL;
        }
        DEBUG_printf("Client connected\n");

        state->client_pcb = client_pcb;
        tcp_arg(client_pcb, state);
        tcp_sent(client_pcb, serverSent);
        tcp_recv(client_pcb, serverReceive);
        tcp_poll(client_pcb, serverPoll, kPollTime * 2);
        tcp_err(client_pcb, serverErr);

        // serverSendData(arg, state->client_pcb, "start\r\n", sizeof "start\r\n");
        // while(true) {
        //     serverSendData(arg, state->client_pcb, "loop\r\n", sizeof "loop\r\n");
        // }
        return ERR_OK;
    }

    static err_t serverPoll(void *arg [[maybe_unused]], struct tcp_pcb *tpcb [[maybe_unused]]) {
        DEBUG_printf("serverPoll\n");
        // return serverResult(arg, -1); // no response is an error?
        struct SliceRet res = log->get();
        
        return serverSendData(arg, stateFromArg(arg)->client_pcb, res.buf, res.len);
        // return serverSendData(arg, stateFromArg(arg)->client_pcb, "wa", sizeof "wa");
    }

    // TODO: Remove probably
    static err_t serverReceive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err [[maybe_unused]]) {

        TCP_SERVER_T *state = stateFromArg(arg);
        if (!p) {
            // return serverResult(arg, -1);
            return serverClose(arg);
        }
        // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
        // can use this method to cause an assertion in debug mode, if this method is called when
        // cyw43_arch_lwip_begin IS needed
        cyw43_arch_lwip_check();
        if (p->tot_len > 0) {

            DEBUG_printf("serverReceive %d/%d err %d\n", p->tot_len, state->recv_len, err);
            // Receive the buffer
            const uint16_t buffer_left = kBufSize - state->recv_len;
            state->recv_len += pbuf_copy_partial(p, state->buffer_recv + state->recv_len,
                                                 p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
            tcp_recved(tpcb, p->tot_len);
        }
        pbuf_free(p);

        // Have we have received the whole buffer
        if (state->recv_len == kBufSize) {

            // check it matches
            if (memcmp(state->buffer_sent, state->buffer_recv, kBufSize) != 0) {
                DEBUG_printf("Buffer mismatch\n");
                // return serverResult(arg, -1);
                return serverClose(arg);
            }

            // Test complete?
            state->run_count++;
            if (state->run_count >= kTestIterations) {
                // serverResult(arg, 0);
                return ERR_OK;
            }

            // Send another buffer
            // return serverSendData(arg, state->client_pcb);
        }
        return ERR_OK;
    }

    static err_t serverSendData(void *arg, struct tcp_pcb *tpcb, const char *s, int len) {

        TCP_SERVER_T *state = stateFromArg(arg);
        // for(int i=0; i< kBufSize; i++) {
        //     state->buffer_sent[i] = 'a';
        // }
        int send_len;
        if(len > kBufSize) {
            DEBUG_printf("Potential overflow happened in serverSendData\n");
            send_len = kBufSize;
        } else {
            send_len = len;
        }
        memcpy(state->buffer_sent, s, send_len);

        state->sent_len = 0;
        DEBUG_printf("Writing %d bytes to client\n", send_len);
        // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
        // can use this method to cause an assertion in debug mode, if this method is called when
        // cyw43_arch_lwip_begin IS needed
        cyw43_arch_lwip_check();
        err_t err = tcp_write(tpcb, state->buffer_sent, send_len, TCP_WRITE_FLAG_COPY);
        if (err != ERR_OK) {
            DEBUG_printf("Failed to write data %d\n", err);
            // return serverResult(arg, -1);
            return serverClose(arg);
        }
        // tcp_output(tpcb);

        return ERR_OK;
    }

    static bool serverOpen(void *arg) {

        TCP_SERVER_T *state = stateFromArg(arg);

        DEBUG_printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), kTcpPort);
        struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
        if(!pcb) {
            DEBUG_printf("Failed to create pcb\n");
            return false;
        }

        err_t err = tcp_bind(pcb, NULL, tcpPort);
        if(err) {
            DEBUG_printf("failed to bind to port %u\n", kTcpPort);
            return false;
        }

        state->server_pcb = tcp_listen_with_backlog(pcb, 1);
        if(!state->server_pcb) {
            DEBUG_printf("Failed to listen\n");
            if(pcb)
                tcp_close(pcb);
            return false;
        }

        tcp_arg(state->server_pcb, state);
        tcp_accept(state->server_pcb, tcpAccept);

        return true;

    }

    static err_t serverClose(void* arg) {
        TCP_SERVER_T *state = stateFromArg(arg);
        err_t err = ERR_OK;
        if (state->client_pcb != NULL) {
            tcp_arg(state->client_pcb, NULL);
            tcp_poll(state->client_pcb, NULL, 0);
            tcp_sent(state->client_pcb, NULL);
            tcp_recv(state->client_pcb, NULL);
            tcp_err(state->client_pcb, NULL);
            err = tcp_close(state->client_pcb);
            if (err != ERR_OK) {
                DEBUG_printf("close failed %d, calling abort\n", err);
                tcp_abort(state->client_pcb);
                err = ERR_ABRT;
            }
            state->client_pcb = NULL;
        }
        if (state->server_pcb) {
            tcp_arg(state->server_pcb, NULL);
            tcp_close(state->server_pcb);
            state->server_pcb = NULL;
        }
        return err;
    }

    static void serverErr(void *arg, err_t err) {
        if (err != ERR_ABRT) {
            DEBUG_printf("tcp_client_err_fn %d\n", err);
            // serverResult(arg, err);
            serverClose(arg);
        }
    }

    // What even is this for
    static err_t serverResult(void *arg, int status) {
        TCP_SERVER_T *state = stateFromArg(arg);
        if (status == 0) {
            DEBUG_printf("test success\n");
        } else {
            DEBUG_printf("test failed %d\n", status);
        }
        state->complete = true;
        return serverClose(arg);
    }

    static err_t serverSent(void *arg, struct tcp_pcb *tpcb [[maybe_unused]], u16_t len) {
        TCP_SERVER_T *state = stateFromArg(arg);
        DEBUG_printf("serverSent %u\n", len);
        state->sent_len += len;

        // if (state->sent_len >= kBufSize) {

        //     // We should get the data back from the client
        //     state->recv_len = 0;
        //     DEBUG_printf("Waiting for buffer from client\n");
        // }

        return ERR_OK;
    }
  public:
    Tcp(LogBuf* l) {

        log = l;
        #ifdef TESTING    
        stdio_init_all();
        #endif

        if(cyw43_arch_init()) {
            DEBUG_printf("Error: cyw43_arch_init failed\n");
            panic("Stopped exec\n");
        }

        cyw43_arch_enable_sta_mode();
        int res = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
        if (res) {
            DEBUG_printf("failed to connect to router %d\n", res);
            panic("Stopped exec");
        } else {
            DEBUG_printf("Connected\n");
        }

        arg = calloc(1, sizeof(TCP_SERVER_T));
        if (!arg) {
            DEBUG_printf("failed to allocate state tcp\n");
            panic("Stopped exec");
        }
    }

    ~Tcp() {

        free(arg);
        cyw43_arch_deinit();
    }
   
    void startServer() {

        if(!serverOpen(arg)) {
            DEBUG_printf("error: serverOpen failed\n");
        }
        // We need this until client connects and enters tcpAccept loop
    }
};
