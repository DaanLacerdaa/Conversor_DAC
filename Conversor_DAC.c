#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "ssd1306/ssd1306.h"

// Definição dos pinos e constantes
#define LED_RED        13
#define LED_GREEN      11
#define LED_BLUE       12
#define JOYSTICK_X     26
#define JOYSTICK_Y     27
#define JOYSTICK_BTN   22
#define BUTTON_A       5
#define I2C_SDA       14
#define I2C_SCL       15
#define PWM_MAX       4095
#define PWM_MIN       50     // Evita brilho muito baixo
#define JOY_CENTER    2048
#define DEBOUNCE_DELAY_MS 50
#define SQUARE_SIZE    8

// Tipos de borda disponíveis
typedef enum {
    BORDER_NONE,
    BORDER_SOLID,
    BORDER_DOUBLE,
    BORDER_ROUND,
    BORDER_DOTTED,
    BORDER_MAX_STYLES
} BorderStyle;

// Variáveis globais
volatile bool joystick_btn_flag = false;
volatile bool button_a_flag = false;
volatile uint64_t last_joy_btn_time = 0;
volatile uint64_t last_a_btn_time = 0;
bool green_led_on = false;
bool pwm_enabled = true;
BorderStyle border_style = BORDER_SOLID;
ssd1306_t display;

// Protótipos de funções
void draw_borders();
void draw_square(uint8_t x, uint8_t y);

// Configuração do ADC
void setup_adc() {
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
}

// Configuração do PWM para os LEDs
void setup_pwm() {
    gpio_set_function(LED_RED, GPIO_FUNC_PWM);
    gpio_set_function(LED_GREEN, GPIO_FUNC_PWM);
    gpio_set_function(LED_BLUE, GPIO_FUNC_PWM);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_MAX);
    pwm_init(pwm_gpio_to_slice_num(LED_RED), &config, true);
    pwm_init(pwm_gpio_to_slice_num(LED_GREEN), &config, true);
    pwm_init(pwm_gpio_to_slice_num(LED_BLUE), &config, true);
}

// Configuração do I2C para o display
void setup_i2c() {
    i2c_init(i2c1, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

// Interrupções com debounce
void joystick_btn_isr(uint gpio, uint32_t events) {
    if (time_us_64() - last_joy_btn_time > DEBOUNCE_DELAY_MS * 1000) {
        joystick_btn_flag = true;
        last_joy_btn_time = time_us_64();
    }
}

void button_a_isr(uint gpio, uint32_t events) {
    if (time_us_64() - last_a_btn_time > DEBOUNCE_DELAY_MS * 1000) {
        button_a_flag = true;
        last_a_btn_time = time_us_64();
    }
}

// Desenha o quadrado móvel
void draw_square(uint8_t x, uint8_t y) {
    ssd1306_draw_square(&display, x, y, SQUARE_SIZE, SQUARE_SIZE);
}

// Desenha bordas com diferentes estilos
void draw_borders() {
    switch (border_style) {
        case BORDER_NONE:
            break;
        case BORDER_SOLID:
            for (int i = 0; i < 128; i++) {
                ssd1306_draw_pixel(&display, i, 0);
                ssd1306_draw_pixel(&display, i, 63);
            }
            for (int i = 0; i < 64; i++) {
                ssd1306_draw_pixel(&display, 0, i);
                ssd1306_draw_pixel(&display, 127, i);
            }
            break;
        case BORDER_DOUBLE:
            for (int i = 1; i < 127; i++) {
                ssd1306_draw_pixel(&display, i, 0);
                ssd1306_draw_pixel(&display, i, 1);
                ssd1306_draw_pixel(&display, i, 62);
                ssd1306_draw_pixel(&display, i, 63);
            }
            for (int i = 1; i < 63; i++) {
                ssd1306_draw_pixel(&display, 0, i);
                ssd1306_draw_pixel(&display, 1, i);
                ssd1306_draw_pixel(&display, 126, i);
                ssd1306_draw_pixel(&display, 127, i);
            }
            break;
    }
}

// Atualiza o display
void update_display(uint16_t x, uint16_t y) {
    ssd1306_clear(&display);
    
    // Corrigindo o mapeamento do joystick (X e Y estavam invertidos)
    uint8_t pos_x = (y * (128 - SQUARE_SIZE)) / PWM_MAX; // Agora usa Y para X
    uint8_t pos_y = (x * (64 - SQUARE_SIZE)) / PWM_MAX;  // Agora usa X para Y
    
    draw_square(pos_x, pos_y);
    draw_borders();
    ssd1306_show(&display);
}

// Loop principal
int main() {
    stdio_init_all();
    setup_adc();
    setup_pwm();
    setup_i2c();

    gpio_set_irq_enabled_with_callback(JOYSTICK_BTN, GPIO_IRQ_EDGE_FALL, true, &joystick_btn_isr);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_a_isr);

    ssd1306_init(&display, 128, 64, 0x3C, i2c1);
    ssd1306_clear(&display);
    ssd1306_show(&display);

    uint16_t x = JOY_CENTER;
    uint16_t y = JOY_CENTER;
    update_display(x, y);

    while (true) {
        adc_select_input(1); // Corrigindo ordem
        x = adc_read();
        adc_select_input(0);
        y = adc_read();

        if (pwm_enabled) {
            uint16_t delta_x = abs(x - JOY_CENTER);
            uint16_t delta_y = abs(y - JOY_CENTER);

            pwm_set_gpio_level(LED_RED, pwm_enabled ? (PWM_MIN + ((delta_x * (PWM_MAX - PWM_MIN)) / JOY_CENTER)) : 0);
            pwm_set_gpio_level(LED_BLUE, pwm_enabled ? (PWM_MIN + ((delta_y * (PWM_MAX - PWM_MIN)) / JOY_CENTER)) : 0);
        } else {
            pwm_set_gpio_level(LED_RED, 0);
            pwm_set_gpio_level(LED_BLUE, 0);
        }

        update_display(x, y);

        if (joystick_btn_flag) {
            joystick_btn_flag = false;
            green_led_on = !green_led_on;
            pwm_set_gpio_level(LED_GREEN, green_led_on ? PWM_MAX : 0);
            border_style = (border_style + 1) % BORDER_MAX_STYLES;
        }

        if (button_a_flag) {
            button_a_flag = false;
            pwm_enabled = !pwm_enabled;
        }

        sleep_ms(50);
    }
}
