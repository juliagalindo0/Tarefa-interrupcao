#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "interrupcao.pio.h"  

// Definição do pino da matriz de LEDs e quantidade total de LEDs
#define LED_MATRIX_PIN 7  
#define TOTAL_LEDS 25  

// Definição dos pinos para botões e LED indicador
#define LED_STATUS 13  // LED indicador do sistema
#define BOTAO_MAIS 5   // Botão para aumentar o número
#define BOTAO_MENOS 6  // Botão para diminuir o número

// Tempo de debounce para evitar múltiplas leituras ao pressionar os botões (200ms)
#define TEMPO_ANTI_REBOTE 200000  

// Variáveis globais de controle
static volatile int numero_atual = 0;  // Número exibido na matriz
static volatile uint32_t ultimo_acionamento = 0;  // Armazena o tempo da última interação do botão
static volatile bool atualizar_matriz = false;  // Indica quando a matriz deve ser atualizada

// Configuração das cores RGB padrão para os LEDs
uint8_t cor_vermelho = 1;  
uint8_t cor_verde = 1;  
uint8_t cor_azul = 1;  

// Padrões de exibição dos números na matriz 5x5 (cada posição representa um LED aceso ou apagado)
bool padrao_numeros[10][TOTAL_LEDS] = {
    { 0, 1, 1, 1, 0,  
      1, 0, 0, 0, 1,  
      1, 0, 0, 0, 1,  
      1, 0, 0, 0, 1,  
      0, 1, 1, 1, 0 },  // Número 0

    { 0, 0, 1, 0, 0,  
      0, 0, 1, 0, 0,  
      0, 0, 1, 0, 0,  
      0, 0, 1, 0, 0,  
      0, 0, 1, 0, 0 },  // Número 1

    { 0, 1, 1, 1, 0,  
      0, 0, 1, 0, 0,  
      0, 1, 0, 0, 0,  
      0, 0, 0, 1, 0,  
      0, 0, 1, 1, 0 },  // Número 2

    { 0, 1, 1, 1, 0,  
      0, 0, 0, 1, 0,  
      0, 1, 1, 1, 0,  
      0, 0, 0, 1, 0,  
      0, 1, 1, 1, 0 },  // Número 3

    { 0, 1, 0, 0, 0,  
      0, 0, 0, 1, 0,  
      0, 1, 1, 1, 0,  
      0, 1, 0, 1, 0,  
      0, 1, 0, 1, 0 },  // Número 4

    { 0, 0, 1, 1, 0,  
      0, 0, 0, 1, 0,  
      0, 0, 1, 1, 0,  
      0, 1, 0, 0, 0,  
      0, 1, 1, 1, 0 },  // Número 5

    { 0, 1, 1, 1, 0,  
      0, 1, 0, 1, 0,  
      0, 1, 1, 1, 0,  
      0, 1, 0, 0, 0,  
      0, 1, 1, 1, 0 },  // Número 6

    { 0, 0, 0, 1, 0,  
      0, 0, 1, 0, 0,  
      0, 0, 1, 0, 0,  
      0, 0, 0, 1, 0,  
      0, 1, 1, 1, 0 },  // Número 7

    { 0, 1, 1, 1, 0,  
      0, 1, 0, 1, 0,  
      0, 1, 1, 1, 0,  
      0, 1, 0, 1, 0,  
      0, 1, 1, 1, 0 },  // Número 8

    { 0, 1, 1, 1, 0,  
      0, 1, 0, 0, 0,  
      0, 1, 1, 1, 0,  
      0, 1, 0, 1, 0,  
      0, 1, 1, 1, 0 }   // Número 9
};

// Converte valores RGB para formato GRB (utilizado pelos LEDs WS2812)
static inline uint32_t converter_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Envia os dados de cor para os LEDs WS2812 através do PIO
static inline void enviar_pixel(uint32_t cor_grb) {
    pio_sm_put_blocking(pio0, 0, cor_grb << 8u);
}

// Atualiza a exibição da matriz de LEDs
void atualizar_display(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t cor = converter_rgb(r, g, b);

    for (int i = 0; i < TOTAL_LEDS; i++) {
        enviar_pixel(padrao_numeros[numero_atual][i] ? cor : 0);
    }
}

// Interrupção para capturar eventos dos botões
void botao_irq_handler(uint gpio, uint32_t events) {
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());

    if (tempo_atual - ultimo_acionamento > TEMPO_ANTI_REBOTE) {
        ultimo_acionamento = tempo_atual;

        if (gpio == BOTAO_MAIS && numero_atual < 9) {
            numero_atual++;
        } else if (gpio == BOTAO_MENOS && numero_atual > 0) {
            numero_atual--;
        }

        atualizar_matriz = true;
    }
}

// Função principal
int main() {   
    stdio_init_all();

    // Inicializa o PIO para controlar a matriz de LEDs
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &interrupcao_program);
    interrupcao_program_init(pio, sm, offset, LED_MATRIX_PIN, 800000, false);
    
    // Configuração dos LEDs e botões
    gpio_init(LED_STATUS);
    gpio_set_dir(LED_STATUS, GPIO_OUT);

    gpio_init(BOTAO_MAIS);
    gpio_set_dir(BOTAO_MAIS, GPIO_IN);
    gpio_pull_up(BOTAO_MAIS);

    gpio_init(BOTAO_MENOS);
    gpio_set_dir(BOTAO_MENOS, GPIO_IN);
    gpio_pull_up(BOTAO_MENOS);

    // Configuração das interrupções dos botões
    gpio_set_irq_enabled_with_callback(BOTAO_MAIS, GPIO_IRQ_EDGE_FALL, true, &botao_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_MENOS, GPIO_IRQ_EDGE_FALL, true, &botao_irq_handler);

    while (true) {
        gpio_put(LED_STATUS, 1);
        sleep_ms(150);
        gpio_put(LED_STATUS, 0);
        sleep_ms(50);

        if (atualizar_matriz) {
            atualizar_display(cor_vermelho, cor_verde, cor_azul);
            atualizar_matriz = false;
        }
    }

    return 0;
}

