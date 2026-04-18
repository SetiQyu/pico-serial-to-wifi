#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "uart.hpp"
#include "tcp.hpp"

int main() {

  static LogBuf log;
  static Uart uart(kPicoTx, kPicoRx, uart_id, kBaudRate);
  static Tcp tcp(&log);
  
  tcp.startServer();
  while(true){
    if(uart_is_readable(uart0))
      log.push(uart.getChar());
  }
  return 0;
}

