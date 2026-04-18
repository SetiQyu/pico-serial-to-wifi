#include "pico/sync.h"

struct SliceRet {
  const char* buf;
  int len;
};

class LogBuf {
  private:
    // 2^16 Chars should be enough for buffer
    // should be fine on both pico 1 and 2
    // static constexpr int bufLimit = 1u << 16;
    static constexpr int bufLimit = 16;
    inline static char buffer[bufLimit];
    inline static int head = 0;
    inline static int lastSent = 0;
    inline static mutex_t mutex;

    void lock() {
      mutex_enter_blocking(&mutex);
    }

    void unlock() {
      mutex_exit(&mutex);
    }

  public:
    LogBuf()
    {
      mutex_init(&mutex);
    }

    void push(const char c) {

        lock();
        
        buffer[head] = c;
        head = (head + 1) % bufLimit;

        unlock();
    }

    struct SliceRet get() {
      
        lock();

        int len;
        int old_sent = lastSent;
        // If buffer has overflowed
        if (head < lastSent) {
          len = bufLimit - lastSent;
          lastSent = 0;
        } else {
          len = head - lastSent;
          lastSent = head;
        }

        unlock();
        return {buffer + old_sent, len};
    }
};
