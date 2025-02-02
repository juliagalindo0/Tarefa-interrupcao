#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "interrupcao.pio.h"

// Definição do número de LEDs na matriz e do pino de controle
#define NUM_PIXELS 25
#define MATRIX_PIN 7

// Definição dos pinos para LED RGB e botões
#define LED_R_PIN 13
#define BUTTON_UP 5
#define BUTTON_DOWN 6

// Tempo mínimo entre interrupções para evitar bouncing (200ms)
#define TEMPO_DEBOUNCE 200000 

// Variáveis globais para controle do número exibido e debounce dos botões
static volatile int current_number = 0;
static volatile uint32_t last_press_time = 0;
static volatile bool refresh_display = false;

// Definição das cores padrão dos LEDs
uint8_t led_r = 1;
uint8_t led_g = 1;
uint8_t led_b = 1;

// Padrões de exibição para os números de 0 a 9 na matriz de LEDs 5x5
bool led_patterns[10][NUM_PIXELS] = {
    { 0, 1, 1, 1, 0,  
      1, 0, 0, 0, 1,  
      1, 0, 0, 0, 1,  
      1, 0, 0, 0, 1,  
      0, 1, 1, 1, 0 }, // 0

    { 0, 0, 1, 0, 0,  
      0, 0, 1, 0, 0,  
      0, 0, 1, 0, 0,  
      0, 0, 1, 0, 0,  
      0, 0, 1, 0, 0 }, // 1

    { 0, 1, 1, 1, 0,  
      0, 0, 1, 0, 0,  
      0, 1, 0, 0, 0,  
      0, 0, 0, 1, 0,  
      0, 0, 1, 1, 0 }, // 2

    { 0, 1, 1, 1, 0,  
      0, 0, 0, 1, 0,  
      0, 1, 1, 1, 0,  
      0, 0, 0, 1, 0,  
      0, 1, 1, 1, 0 }, // 3

    { 0, 1, 0, 0, 0,  
      0, 0, 0, 1, 0,  
      0, 1, 1, 1, 0,  
      0, 1, 0, 1, 0,  
      0, 1, 0, 1, 0 }, // 4

    { 0, 0, 1, 1, 0,  
      0, 0, 0, 1, 0,  
      0, 0, 1, 1, 0,  
      0, 1, 0, 0, 0,  
      0, 1, 1, 1, 0 }, // 5

    { 0, 1, 1, 1, 0,  
      0, 1, 0, 1, 0,  
      0, 1, 1, 1, 0,  
      0, 1, 0, 0, 0,  
      0, 1, 1, 1, 0 }, // 6

    { 0, 0, 0, 1, 0,  
      0, 0, 1, 0, 0,  
      0, 0, 1, 0, 0,  
      0, 0, 0, 1, 0,  
      0, 1, 1, 1, 0 }, // 7

    { 0, 1, 1, 1, 0,  
      0, 1, 0, 1, 0,  
      0, 1, 1, 1, 0,  
      0, 1, 0, 1, 0,  
      0, 1, 1, 1, 0 }, // 8

    { 0, 1, 1, 1, 0,  
      0, 1, 0, 0, 0,  
      0, 1, 1, 1, 0,  
      0, 1, 0, 1, 0,  
      0, 1, 1, 1, 0 }  // 9
};

// Envia um pixel para o LED interrupcao via PIO
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Converte valores RGB para formato GRB
static inline uint32_t convert_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Atualiza a matriz de LEDs com base no número atual
void update_led_matrix(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = convert_rgb(r, g, b);

    for (int i = 0; i < NUM_PIXELS; i++) {
        if (led_patterns[current_number][i]) {
            put_pixel(color);
        } else {
            put_pixel(0);
        }
    }
}

// Interrupção dos botões para incrementar/decrementar números
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (current_time - last_press_time > TEMPO_DEBOUNCE) {
        last_press_time = current_time;

        if (gpio == BUTTON_UP && current_number < 9) {
            current_number++;
        } else if (gpio == BUTTON_DOWN && current_number > 0) {
            current_number--;
        }

        refresh_display = true;
    }
}

// Função principal
int main() {   
    stdio_init_all();

    // Inicializa o PIO para controlar a matriz interrupcao
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &interrupcao_program);
    interrupcao_program_init(pio, sm, offset, MATRIX_PIN, 800000, false);
    
    // Configuração dos LEDs e botões
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);

    gpio_init(BUTTON_UP);
    gpio_set_dir(BUTTON_UP, GPIO_IN);
    gpio_pull_up(BUTTON_UP);

    gpio_init(BUTTON_DOWN);
    gpio_set_dir(BUTTON_DOWN, GPIO_IN);
    gpio_pull_up(BUTTON_DOWN);

    // Configuração das interrupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_UP, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_DOWN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicializa a matriz de LEDs com o primeiro número
    update_led_matrix(led_r, led_g, led_b);

    // Loop principal
    while (true) {
        gpio_put(LED_R_PIN, 1);
        sleep_ms(150);
        gpio_put(LED_R_PIN, 0);
        sleep_ms(50);

        if (refresh_display) {
            update_led_matrix(led_r, led_g, led_b);
            refresh_display = false;
        }
    }

    return 0;
}
