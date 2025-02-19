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
#define I2C_SDA        14
#define I2C_SCL        15
#define PWM_MAX        4095
#define PWM_MIN        50     // Evita brilho muito baixo
#define JOY_CENTER     2048
#define DEBOUNCE_DELAY_MS 50
#define SQUARE_SIZE    8
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64


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

// Variáveis para o quadrado no display
int square_x = (SCREEN_WIDTH - SQUARE_SIZE) / 2;
int square_y = (SCREEN_HEIGHT - SQUARE_SIZE) / 2;

// Protótipos de funções
void draw_borders();
void draw_square(uint8_t x, uint8_t y);
void update_display();
void setup_adc();
void setup_pwm();
void setup_i2c();
void joystick_btn_isr(uint gpio, uint32_t events);
void button_a_isr(uint gpio, uint32_t events);

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
            for (int i = 0; i < SCREEN_WIDTH; i++) {
                ssd1306_draw_pixel(&display, i, 0);
                ssd1306_draw_pixel(&display, i, SCREEN_HEIGHT - 1);
            }
            for (int i = 0; i < SCREEN_HEIGHT; i++) {
                ssd1306_draw_pixel(&display, 0, i);
                ssd1306_draw_pixel(&display, SCREEN_WIDTH - 1, i);
            }
            break;
    }
}

// Atualiza o display
void update_display() {
    ssd1306_clear(&display);
    draw_square(square_x, square_y);
    draw_borders();
    ssd1306_show(&display);
}

// Loop principal
int main() {
    stdio_init_all();
    setup_adc();
    setup_pwm();
    setup_i2c();
    gpio_pull_up(JOYSTICK_BTN);
    gpio_pull_up(BUTTON_A);

    gpio_set_irq_enabled_with_callback(JOYSTICK_BTN, GPIO_IRQ_EDGE_FALL, true, &joystick_btn_isr);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_a_isr);

    ssd1306_init(&display, SCREEN_WIDTH, SCREEN_HEIGHT, 0x3C, i2c1);
    ssd1306_clear(&display);
    ssd1306_show(&display);

    while (true) {
        // **Leitura correta dos eixos do joystick**
        adc_select_input(0);
        uint16_t x = adc_read();
        adc_select_input(1);
        uint16_t y = adc_read();

        // **Mapeamento correto**
        square_x = (x * (SCREEN_WIDTH - SQUARE_SIZE)) / 4095;
        square_y = (y * (SCREEN_HEIGHT - SQUARE_SIZE)) / 4095;

        // **Controle PWM corrigido**
        if (pwm_enabled) {
            pwm_set_gpio_level(LED_BLUE, abs((int)y - JOY_CENTER) / 16);
            pwm_set_gpio_level(LED_RED, abs((int)x - JOY_CENTER) / 16);
        } else {
            pwm_set_gpio_level(LED_BLUE, 0);
            pwm_set_gpio_level(LED_RED, 0);
        }

        update_display();

        // **Botão do Joystick altera a borda**
        if (joystick_btn_flag) {
            joystick_btn_flag = false;
            green_led_on = !green_led_on;
            pwm_set_gpio_level(LED_GREEN, green_led_on ? PWM_MAX : 0);
            border_style = (border_style + 1) % BORDER_MAX_STYLES;
            update_display();
        }

        // **Botão A desliga os LEDs**
        if (button_a_flag) {
            button_a_flag = false;
            pwm_enabled = !pwm_enabled;
            if (!pwm_enabled) {
                pwm_set_gpio_level(LED_RED, 0);
                pwm_set_gpio_level(LED_BLUE, 0);
                pwm_set_gpio_level(LED_GREEN, 0);
            }
        }

        sleep_ms(50);
    }
}
