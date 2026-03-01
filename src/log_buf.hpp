#include "misc.hpp"
struct SliceRet {
  const char* buf;
  int len;
};

class LogBuf {
  private:
    // 2^16 Chars should be enough for buffer
    // should be fine on both pico 1 and 2
    static constexpr int bufLimit = 1u << 16;
    inline static char buffer[bufLimit];
    inline static int head = 0;
    inline static int lastSent = 0;
    static Led led;

  public:
    void push(const char c) {
        led.ledSet(true);
        buffer[head] = c;
        head = (head + 1) % bufLimit;
    }

    struct SliceRet get() {
      
        int len;
        int old_sent = lastSent;
        // If buffer has overflowed
        if (head < lastSent) {
          led.ledSet(false);
          len = bufLimit - lastSent;
          lastSent = 0;
        } else {
          len = head - lastSent;
          lastSent = head;
        }
        // for(int i = 0; i < len; i++)
        //   printf("%c", (buffer + old_sent)[i]);
        
        return {buffer + old_sent, len};
    }
};
