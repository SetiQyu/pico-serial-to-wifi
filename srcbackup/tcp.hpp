/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/cyw43_arch.h"
#include <stdlib.h>

#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
constexpr int kTcpPort = 4242;
constexpr int kBufSize = 2048;
constexpr int kTestIterations = 10;
constexpr int kPollTime = 5;


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
    // static Uart uart;
    inline static int tcpPort = kTcpPort;
    TCP_SERVER_T *kstate;

    static err_t tcpAccept(void *arg, struct tcp_pcb *client_pcb, err_t err) {

      TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
      if (err != ERR_OK || client_pcb == NULL) {
    //       uart.putString(CRLF("Failure in accept\n"));
          serverResult(arg, err);
          return ERR_VAL;
      }
    //   uart.putString(CRLF("Client connected\n"));

      state->client_pcb = client_pcb;
      tcp_arg(client_pcb, state);
      tcp_sent(client_pcb, serverSent);
      tcp_recv(client_pcb, serverReceive);
      tcp_poll(client_pcb, serverPoll, kPollTime * 2);
      tcp_err(client_pcb, serverErr);

      return serverSendData(arg, state->client_pcb);
    }
    
    static err_t serverPoll(void *arg, struct tcp_pcb *tpcb [[maybe_unused]]) {
    //     uart.putString(CRLF("tcp_server_poll_fn"));
        return serverResult(arg, -1); // no response is an error?
    }

    static err_t serverReceive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err [[maybe_unused]]) {
        TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
        if (!p) {
            return serverResult(arg, -1);
        }
        // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
        // can use this method to cause an assertion in debug mode, if this method is called when
        // cyw43_arch_lwip_begin IS needed
        cyw43_arch_lwip_check();
        if (p->tot_len > 0) {
    //         uart.putString(CRLF("tcp_server_recv"));

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
    //             uart.putString(CRLF("buffer mismatch"));
                return serverResult(arg, -1);
            }
    //         uart.putString(CRLF("tcp_server_recv buffer ok"));

            // Test complete?
            state->run_count++;
            if (state->run_count >= kTestIterations) {
                serverResult(arg, 0);
                return ERR_OK;
            }

            // Send another buffer
            return serverSendData(arg, state->client_pcb);
        }
        return ERR_OK;
    }

    static err_t serverSendData(void *arg, struct tcp_pcb *tpcb)
    {
        TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
        for(int i=0; i< kBufSize; i++) {
            state->buffer_sent[i] = rand();
        }

        state->sent_len = 0;
    //     uart.putString(CRLF("Writing bytes to client\n"));
        // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
        // can use this method to cause an assertion in debug mode, if this method is called when
        // cyw43_arch_lwip_begin IS needed
        cyw43_arch_lwip_check();
        err_t err = tcp_write(tpcb, state->buffer_sent, kBufSize, TCP_WRITE_FLAG_COPY);
        if (err != ERR_OK) {
    //         uart.putString(CRLF("Failed to write data"));
            return serverResult(arg, -1);
        }
        return ERR_OK;
    }
    
    static bool serverConnect(void *arg) {

        TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
        
    //     uart.putString(ip4addr_ntoa(netif_ip4_addr(netif_list)));
    //     uart.putString("\r\n");
        struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
        if(!pcb) {
    //     uart.putString(CRLF("failed to create pcb"));
        return false;
        }

        err_t err = tcp_bind(pcb, NULL, tcpPort);
        if(err) {
    //       uart.putString(CRLF("failed to bind to port %u\n"));
          return false;
        }

        state->server_pcb = tcp_listen_with_backlog(pcb, 1);
        if(!state->server_pcb) {
    //       uart.putString(CRLF("failed to listen"));
          if(pcb) 
              tcp_close(pcb);
          return false;
        }

        tcp_arg(state->server_pcb, state);
        tcp_accept(state->server_pcb, tcpAccept);

        return true;
     
    }

    static err_t serverClose(void* arg) {
        TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
        err_t err = ERR_OK;
        if (state->client_pcb != NULL) {
            tcp_arg(state->client_pcb, NULL);
            tcp_poll(state->client_pcb, NULL, 0);
            tcp_sent(state->client_pcb, NULL);
            tcp_recv(state->client_pcb, NULL);
            tcp_err(state->client_pcb, NULL);
            err = tcp_close(state->client_pcb);
            if (err != ERR_OK) {
    //             uart.putString(CRLF("close failed %d, calling abort"));
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
    //       uart.putString("tcp_client_err_fn");
          serverResult(arg, err);
      }
    }

    static err_t serverResult(void *arg, int status) {
        TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
        if (status == 0) {
    //         uart.putString(CRLF("test success"));
        } else {
    //         uart.putString(CRLF("test failed"));
        }
        state->complete = true;
        return serverClose(arg);
    }
    static err_t serverSent(void *arg, struct tcp_pcb *tpcb [[maybe_unused]], u16_t len) {
          TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
          state->sent_len += len;

          if (state->sent_len >= kBufSize) {

              // We should get the data back from the client
              state->recv_len = 0;
          }

          return ERR_OK;
      }
  public:
    Tcp()
    {
      
      if(cyw43_arch_init()) {
    //       uart.putString(CRLF("Error: cyw43_arch_init failed\n"));
          panic("Stopped exec\n");
      }

      int res = cyw43_arch_wifi_connect_timeout_ms("NETGEAR92", "jaggedmountain289", CYW43_AUTH_WPA2_MIXED_PSK, 30000);
      // int res = cyw43_arch_wifi_connect_blocking("NETGEAR92", "jaggedmountain289", CYW43_AUTH_WPA2_MIXED_PSK);
      if (res) {
    //         uart.putString(CRLF("failed to connect."));
    //         uart.putInt(res);
            panic("Stopped exec");
        } else {
    //         uart.putString("Connected.\n");
        }
          

      kstate = static_cast<TCP_SERVER_T*>(calloc(1, sizeof(TCP_SERVER_T)));
      if (!kstate) {
    //       uart.putString(CRLF("failed to allocate state tcp"));
          panic("Stopped exec");
      }
    }

    ~Tcp()
    {
      // lwip_nosys_deinit(&async.core);
      // async_context_deinit(&async.core);
      free(kstate);
      cyw43_arch_deinit();
    }

    void startServer() {
      
      if(!serverConnect(kstate)) {
    //     uart.putString(CRLF("error: serverConnect failed\n"));
        return;
      }
      
      while(!kstate->complete) {
        sleep_ms(1000);
      }
      
    }
    
};
