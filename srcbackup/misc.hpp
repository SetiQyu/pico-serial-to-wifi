constexpr int kLedOut = 20;

class Led {
    private:
        int pin;
    public:
        Led(int p) : pin(p) {
            gpio_init(pin);
            gpio_set_dir(pin, GPIO_OUT);
        }
        void ledSet(bool b) {
            gpio_put(pin, b);
        }
};
