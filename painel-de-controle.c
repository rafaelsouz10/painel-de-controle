#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// DEFINIÇÕES
#define BOTAO_ENTRADA 5
#define BOTAO_SAIDA 6
#define BOTAO_RESET 22
#define MAX_USUARIOS  8

// SEMÁFOROS E MUTEX 
SemaphoreHandle_t xSemaforoContagem; // Semáforo de contagem para controle de vagas
SemaphoreHandle_t xSemaforoReset;    // Semáforo binário para reset via interrupção

// VARIÁVEIS DO SISTEMA
volatile uint8_t usuariosAtivos = 0; // Contador de usuários ativos

// TASKS LIBS
#include "lib/config_buzzer.h"
#include "lib/config_leds.h"


//TASK DE ENTRADA
void vTaskEntrada(void *params) {
    bool botaoAnterior = true;

    while (true) {
        bool botaoAtual = gpio_get(BOTAO_ENTRADA);

        // Detecta borda de descida (pressionado)
        if (!botaoAtual && botaoAnterior) {
            if (uxSemaphoreGetCount(xSemaforoContagem) < MAX_USUARIOS) {
                xSemaphoreGive(xSemaforoContagem);
                usuariosAtivos++;
                atualizarLedRGB(usuariosAtivos);

                printf("[ENTRADA] Usuario entrou. Total: %d\n", usuariosAtivos);
            } else {
                // Capacidade cheia – Beep curto
                buzzer_start_alarm();
                vTaskDelay(pdMS_TO_TICKS(200));
                buzzer_stop_alarm();

                printf("[ENTRADA] Capacidade maxima atingida (%d)\n", MAX_USUARIOS);
            }

            vTaskDelay(pdMS_TO_TICKS(300));
        }

        botaoAnterior = botaoAtual;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#define BOTAO_SAIDA 6

void vTaskSaida(void *params) {
    bool botaoAnterior = true;

    while (true) {
        bool botaoAtual = gpio_get(BOTAO_SAIDA);

        if (!botaoAtual && botaoAnterior) {
            if (usuariosAtivos > 0) {
                // Tenta "pegar" uma vaga do semáforo
                if (xSemaphoreTake(xSemaforoContagem, pdMS_TO_TICKS(100)) == pdTRUE) {
                    usuariosAtivos--;
                    atualizarLedRGB(usuariosAtivos);
                    printf("[SAIDA] Usuario saiu. Total: %d\n", usuariosAtivos);
                }
            } else {
                printf("[SAIDA] Nenhum usuario presente.\n");
            }

            vTaskDelay(pdMS_TO_TICKS(300));
        }

        botaoAnterior = botaoAtual;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void gpio_callback(uint gpio, uint32_t events) {
    volatile uint32_t last_time = 0;    // Armazena o tempo do último evento (em microssegundos)
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Obtém o tempo atual em microssegundos

    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > 300000) { // 300 ms de debouncing
        last_time = current_time; // Atualiza o tempo do último evento

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        xSemaphoreGiveFromISR(xSemaforoReset, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void vTaskReset(void *params) {
    while (true) {
        if (xSemaphoreTake(xSemaforoReset, portMAX_DELAY) == pdTRUE) {
            usuariosAtivos = 0; // Reinicia variáveis e semáforo de contagem
            atualizarLedRGB(usuariosAtivos);

            // Zera o semáforo de contagem recriando
            vSemaphoreDelete(xSemaforoContagem);
            xSemaforoContagem = xSemaphoreCreateCounting(MAX_USUARIOS, 0);

            // Beep duplo
            for (int i = 0; i < 2; i++) {
                buzzer_start_alarm();
                vTaskDelay(pdMS_TO_TICKS(150));
                buzzer_stop_alarm();
                vTaskDelay(pdMS_TO_TICKS(150));
            }

            printf("[RESET] Sistema reiniciado. Total de usuarios: 0\n");
        }
    }
}


int main() {
    stdio_init_all();

    // Inicializa botão de entrada
    gpio_init(BOTAO_ENTRADA);
    gpio_set_dir(BOTAO_ENTRADA, GPIO_IN);
    gpio_pull_up(BOTAO_ENTRADA);

    // Inicializa botão de saída
    gpio_init(BOTAO_SAIDA);
    gpio_set_dir(BOTAO_SAIDA, GPIO_IN);
    gpio_pull_up(BOTAO_SAIDA);

    // Inicializa botão de reset
    gpio_init(BOTAO_RESET);
    gpio_set_dir(BOTAO_RESET, GPIO_IN);
    gpio_pull_up(BOTAO_RESET);
    gpio_set_irq_enabled_with_callback(BOTAO_RESET, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);


    // Inicializa buzzer
    buzzer_init();

    // Inicializa Leds RGB
    leds_init();

    // Cria semáforo de contagem
    xSemaforoContagem = xSemaphoreCreateCounting(MAX_USUARIOS, 0);

    // Cria a tarefa de entrada
    xTaskCreate(vTaskEntrada, "Entrada", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "Saida", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);

        
    xSemaforoReset = xSemaphoreCreateBinary(); // Cria semáforo binário para reset
    xTaskCreate(vTaskReset, "Reset", configMINIMAL_STACK_SIZE + 128, NULL, 2, NULL); // Cria a task de reset


    // Inicia o agendador do FreeRTOS
    vTaskStartScheduler();
}
