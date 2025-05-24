#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// DEFINIÇÕES
#define BOTAO_ENTRADA 6
#define BOTAO_SAIDA 5
#define BOTAO_RESET 22
#define MAX_USUARIOS  8

static volatile uint32_t last_time = 0;    // Armazena o tempo do último evento (em microssegundos)

// SEMÁFOROS E MUTEX 
SemaphoreHandle_t xSemaforoContagem; // Semáforo de contagem para controle de vagas
SemaphoreHandle_t xSemaforoReset;    // Semáforo binário para reset via interrupção
SemaphoreHandle_t xMutexDisplay;

// VARIÁVEIS DO SISTEMA
volatile uint8_t usuariosAtivos = 0; // Contador de usuários ativos

// LIBS CONFIG E TASKS
#include "lib/config_buzzer.h"
#include "lib/config_leds.h"
#include "lib/config_display.h"

//TASK DE ENTRADA
void vTaskEntrada(void *params) {
    // Inicializa botão de entrada
    gpio_init(BOTAO_ENTRADA); gpio_set_dir(BOTAO_ENTRADA, GPIO_IN); gpio_pull_up(BOTAO_ENTRADA);

    bool botaoAnterior = true;
    char buffer[32];

    while (true) {
        bool botaoAtual = gpio_get(BOTAO_ENTRADA);

        // Detecta borda de descida (pressionado)
        if (!botaoAtual && botaoAnterior) {
            if (uxSemaphoreGetCount(xSemaforoContagem) < MAX_USUARIOS) {
                xSemaphoreGive(xSemaforoContagem);
                usuariosAtivos++;
                atualizarLedRGB(usuariosAtivos);

                printf("[ENTRADA] Usuario entrou. Total: %d\n", usuariosAtivos);

                if (xSemaphoreTake(xMutexDisplay, pdMS_TO_TICKS(100))) {
                    ssd1306_fill(&ssd, 0);
                    ssd1306_draw_string(&ssd, "Entrada", 0, 0);
                    ssd1306_draw_string(&ssd, "autorizada", 0, 10);
                    sprintf(buffer, "Usuarios: %d", usuariosAtivos);
                    ssd1306_draw_string(&ssd, buffer, 0, 34);
                    ssd1306_send_data(&ssd);
                    xSemaphoreGive(xMutexDisplay);
                }                
            } else {
                // Capacidade cheia – Beep curto
                buzzer_start_alarm();
                vTaskDelay(pdMS_TO_TICKS(200));
                buzzer_stop_alarm();

                printf("[ENTRADA] Capacidade maxima atingida (%d)\n", MAX_USUARIOS);

                if (xSemaphoreTake(xMutexDisplay, pdMS_TO_TICKS(100))) {
                    ssd1306_fill(&ssd, 0);
                    ssd1306_draw_string(&ssd, "Lotado!", 0, 16);
                    ssd1306_draw_string(&ssd, "Aguarde...", 0, 34);
                    sprintf(buffer, "Usuarios: %d", usuariosAtivos);
                    ssd1306_draw_string(&ssd, buffer, 0, 54);
                    ssd1306_send_data(&ssd);
                    xSemaphoreGive(xMutexDisplay);
                }
            }

            vTaskDelay(pdMS_TO_TICKS(300));
        }

        botaoAnterior = botaoAtual;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vTaskSaida(void *params) {
    // Inicializa botão de saída
    gpio_init(BOTAO_SAIDA); gpio_set_dir(BOTAO_SAIDA, GPIO_IN); gpio_pull_up(BOTAO_SAIDA);

    bool botaoAnterior = true;
    char buffer[32];

    while (true) {
        bool botaoAtual = gpio_get(BOTAO_SAIDA);

        if (!botaoAtual && botaoAnterior) {
            if (usuariosAtivos > 0) {
                // Tenta "pegar" uma vaga do semáforo
                if (xSemaphoreTake(xSemaforoContagem, pdMS_TO_TICKS(100)) == pdTRUE) {
                    usuariosAtivos--;
                    atualizarLedRGB(usuariosAtivos);

                    printf("[SAIDA] Usuario saiu. Total: %d\n", usuariosAtivos);

                    if (xSemaphoreTake(xMutexDisplay, pdMS_TO_TICKS(100))) {
                        ssd1306_fill(&ssd, 0);
                        ssd1306_draw_string(&ssd, "Saida", 0, 0);
                        ssd1306_draw_string(&ssd, "realizada", 0, 10);
                        sprintf(buffer, "Usuarios: %d", usuariosAtivos);
                        ssd1306_draw_string(&ssd, buffer, 0, 34);
                        ssd1306_send_data(&ssd);
                        xSemaphoreGive(xMutexDisplay);
                    }
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
    // Inicializa botão de reset
    gpio_init(BOTAO_RESET); gpio_set_dir(BOTAO_RESET, GPIO_IN); gpio_pull_up(BOTAO_RESET);

    char buffer[32];

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

            if (xSemaphoreTake(xMutexDisplay, pdMS_TO_TICKS(100))) {
                ssd1306_fill(&ssd, 0);
                ssd1306_draw_string(&ssd, "Sistema", 0, 0);
                ssd1306_draw_string(&ssd, "Resetado", 0, 16);
                sprintf(buffer, "Usuarios: 0");
                ssd1306_draw_string(&ssd, buffer, 0, 34);
                ssd1306_send_data(&ssd);
                xSemaphoreGive(xMutexDisplay);
            }
        }
    }
}


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
