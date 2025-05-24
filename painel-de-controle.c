#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define MAX_USUARIOS  8 // Define a capacidade máxima do sistema

// SEMÁFOROS E MUTEX 
SemaphoreHandle_t xSemaforoContagem;  // Semáforo de contagem para controlar vagas disponíveis
SemaphoreHandle_t xSemaforoReset;    // Semáforo binário para sinalizar reset via interrupção
SemaphoreHandle_t xMutexDisplay;    // Mutex para garantir acesso exclusivo ao display OLED

volatile uint8_t usuariosAtivos = 0; // Variável global com o número atual de usuários ativos

// LIBS CONFIG E TASKS
#include "lib/config_buzzer.h"   // Inicialização e controle do buzzer
#include "lib/config_leds.h"    // Inicialização e controle do LED RGB
#include "lib/config_display.h"// Inicialização e manipulação do display OLED
#include "lib/task_entrada.h" // Task responsável por controlar a entrada
#include "lib/task_saida.h"  // Task responsável por controlar a saída
#include "lib/task_reset.h" // Task e callback para reset do sistema

int main() {
    stdio_init_all();
    buzzer_init();    // Inicializa buzzer
    leds_init();     // Inicializa Leds RGB
    display_init(); // Inicializa o Display

    gpio_set_irq_enabled_with_callback(BOTAO_RESET, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Cria semáforo de contagem
    xSemaforoContagem = xSemaphoreCreateCounting(MAX_USUARIOS, 0); // Contador de ocupação
    xSemaforoReset = xSemaphoreCreateBinary();                    // Binário para reset
    xMutexDisplay = xSemaphoreCreateMutex();                     // Mutex para proteger OLED

    display_start(); // Exibe a mensagem inicial no OLED

    // Cria a tarefa de entrada
    xTaskCreate(vTaskEntrada, "Entrada", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "Saida", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "Reset", configMINIMAL_STACK_SIZE + 128, NULL, 2, NULL);

    vTaskStartScheduler(); // Inicia o escalonador multitarefa
}
