# Tarefa-interrupcao
A presente tarefa tem como objetivo consolidar a compreensão dos conceitos relacionados ao uso de interrupções no microcontrolador RP2040 e explorar as funcionalidades da placa de desenvolvimento BitDogLab .Este projeto implementa o controle de uma matriz de LEDs 5x5 usando um Raspberry Pi Pico (RP2040). Ele permite a exibição de números de 0 a 9, utilizando botões físicos para incrementar e decrementar os valores, tudo controlado por interrupções para garantir eficiência e responsividade.

📌 Funcionalidades

Utiliza interrupções para detecção dos botões.

Implementa debouncing via software para evitar leituras incorretas.

Exibe números de 0 a 9 na matriz de LEDs 5x5.

O LED indicador pisca para mostrar que o sistema está ativo.

Utiliza PIO (Programmable I/O) para comunicação eficiente com a matriz de LEDs WS2812.

🛠 Tecnologias Utilizadas

Linguagem: C

Plataforma: Raspberry Pi Pico (RP2040)

Bibliotecas:

pico/stdlib.h (Funções padrão do RP2040)

hardware/pio.h (Manipulação do PIO)

hardware/clocks.h (Configuração de clocks do microcontrolador)

🏰 Hardware Necessário

Raspberry Pi Pico (RP2040)

Matriz de LEDs WS2812 (5x5)

Botão de incremento (GPIO 5)

Botão de decremento (GPIO 6)

LED indicador (opcional) (GPIO 13)

📚 Funcionamento

O LED indicador pisca a cada 200ms para indicar que o sistema está rodando.

Pressionar o botão de incremento (GPIO 5) aumenta o número exibido na matriz de LEDs.

Pressionar o botão de decremento (GPIO 6) reduz o número exibido na matriz.

O código PIO (interrupcao.pio) controla os LEDs WS2812, garantindo atualizações rápidas.
