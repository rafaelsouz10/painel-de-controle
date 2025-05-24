#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define MAX_USUARIOS  8

// SEMÁFOROS E MUTEX 
SemaphoreHandle_t xSemaforoContagem;  // Semáforo de contagem para controle de vagas
SemaphoreHandle_t xSemaforoReset;    // Semáforo binário para reset via interrupção
SemaphoreHandle_t xMutexDisplay;

// VARIÁVEIS DO SISTEMA
volatile uint8_t usuariosAtivos = 0; // Contador de usuários ativos

// LIBS CONFIG E TASKS
#include "lib/config_buzzer.h"
#include "lib/config_leds.h"
#include "lib/config_display.h"
#include "lib/task_entrada.h"
#include "lib/task_saida.h"
#include "lib/task_reset.h"

int main() {
    stdio_init_all();
    buzzer_init();    // Inicializa buzzer
    leds_init();     // Inicializa Leds RGB
    display_init(); // Inicializa o Display

    gpio_set_irq_enabled_with_callback(BOTAO_RESET, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Cria semáforo de contagem
    xSemaforoContagem = xSemaphoreCreateCounting(MAX_USUARIOS, 0);
    xSemaforoReset = xSemaphoreCreateBinary(); // Cria semáforo binário para reset
    xMutexDisplay = xSemaphoreCreateMutex();

    display_start(); // Inicia com uma mensagem

    // Cria a tarefa de entrada
    xTaskCreate(vTaskEntrada, "Entrada", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "Saida", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "Reset", configMINIMAL_STACK_SIZE + 128, NULL, 2, NULL); // Cria a task de reset

    // Inicia o agendador do FreeRTOS
    vTaskStartScheduler();
}
