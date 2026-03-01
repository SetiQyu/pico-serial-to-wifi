#include "hardware/uart.h"

constexpr int kPicoTx = 16;
constexpr int kPicoRx = 17;
constexpr int kBaudRate = 115200;
uart_inst_t *uart_id = uart0;
#define CRLF(s) s "\r\n"

class Uart {
  private:
    int txd;
    int rxd;
    uart_inst_t *uart_id;
    int baud_rate;
  public:
    Uart(int t, int r, uart_inst_t *ui, int bt)
        : txd(t), rxd(r), uart_id(ui), baud_rate(bt) {
        uart_init(uart_id, baud_rate);
        gpio_set_function(txd, UART_FUNCSEL_NUM(uart_id, txd));
        gpio_set_function(rxd, UART_FUNCSEL_NUM(uart_id, rxd));
    }

    ~Uart() {
        uart_deinit(uart_id);
    }

    char getChar() const {
        return uart_getc(uart_id);
    }

    void putChar(const char c) const {
        uart_putc(uart_id, c);
    }

    void putString(const char* s) const {
        uart_puts(uart_id, s);
    }

    void putInt(const int i) const {
        switch(i) {
        case 0:
            putString(CRLF("0"));
            break;
        case 1:
            putString(CRLF("1"));
            break;
        case 2:
            putString(CRLF("2"));
            break;
        case 3:
            putString(CRLF("3"));
            break;
        case 4:
            putString(CRLF("4"));
            break;
        case 5:
            putString(CRLF("5"));
            break;
        case 6:
            putString(CRLF("6"));
            break;
        case 7:
            putString(CRLF("7"));
            break;
        case 8:
            putString(CRLF("8"));
            break;
        case 9:
            putString(CRLF("9"));
            break;
        case -1:
            putString(CRLF("-1"));
            break;
        case -2:
            putString(CRLF("-2"));
            break;
        case -3:
            putString(CRLF("-3"));
            break;
        case -4:
            putString(CRLF("-4"));
            break;
        case -5:
            putString(CRLF("-5"));
            break;
        case -6:
            putString(CRLF("-6"));
            break;
        case -7:
            putString(CRLF("-7"));
            break;
        case -8:
            putString(CRLF("-8"));
            break;
        case -9:
            putString(CRLF("-9"));
            break;
        default:
            putString(CRLF("Outside range"));
            break;
        }
    }

};
