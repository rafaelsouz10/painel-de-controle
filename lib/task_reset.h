#ifndef TASK_RESET_H
#define TASK_RESET_H

static volatile uint32_t last_time = 0;    // Armazena o tempo do último evento (em microssegundos)

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
                ssd1306_draw_string(&ssd, "SISTEMA", 0, 0);
                ssd1306_draw_string(&ssd, "RESETADO", 0, 10);
                sprintf(buffer, "Usuarios: 0");
                ssd1306_draw_string(&ssd, buffer, 0, 34);
                ssd1306_send_data(&ssd);
                xSemaphoreGive(xMutexDisplay);
            }
        }
    }
}

#endif