// #include "<stdio.h>"

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "uart.hpp"
#include "tcp.hpp"
// #include "misc.hpp"
// #include "log_buf.hpp"


Led LogBuf::led(kLedOut);

int main() {

  LogBuf log;
  // Led led(kLedOut);
  static Uart uart(kPicoTx, kPicoRx, uart_id, kBaudRate);
  Tcp tcp(&log);
  
  tcp.startServer();
  while(true){
    if(uart_is_readable(uart0)) {
      char c = uart.getChar();
      log.push(c);
      // struct SliceRet res = log.get();
      // for(int i = 0; i < res.len; i++)
      //   uart.putChar(res.buf[i]);
      // if(c == 'o')
      //   led.ledSet(true);
      // if(c == 'p')
      //   led.ledSet(false);

    }
      // TODO: use the below instead we want to register interrupts
      // __wfi();
  }
  return 0;
}

