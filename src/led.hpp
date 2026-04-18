class Led {
  private:
    int pin;
    int blinkRate;
  public:
    Led(int p, int b) : pin(p), blinkRate(b) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }
    void ledSet(bool b) {
        gpio_put(pin, b);
    }
};
