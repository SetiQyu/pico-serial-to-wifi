// #include "<stdio.h>"

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "uart.hpp"
#include "tcp.hpp"


class LogBuf {
  private:
    // 2^16 Chars should be enough for buffer
    // should be fine on both pico 1 and 2
    static constexpr int buf_limit = 1u << 16;
    inline static char buffer[buf_limit];
    inline static int head = 0;
    inline static int last_sent = 0;

  public:
    inline void push(const char c){
        buffer[head] = c;
        head = (head + 1) % buf_limit;
    }

    // inline struct SliceRet get() {
    //     int len = head - last_sent;
    //     last_sent = len;
    //     return {&buffer + (head * sizeof(char)), len};
    // }
};


// Uart Tcp::uart(kPicoTx, kPicoRx, uart_id, kBaudRate);

int main() {

    stdio_init_all();
    printf("test\n");
    Tcp tcp;

    tcp.startServer();
    
    return 0;
}

