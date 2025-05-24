#ifndef TASK_SAIDA_H
#define TASK_SAIDA_H

#define BOTAO_SAIDA 5
#define BOTAO_RESET 22

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
                        ssd1306_draw_string(&ssd, "SAIDA", 0, 0);
                        ssd1306_draw_string(&ssd, "REALIZADA", 0, 10);
                        sprintf(buffer, "Usuarios: %d", usuariosAtivos);
                        ssd1306_draw_string(&ssd, buffer, 0, 34);
                        ssd1306_send_data(&ssd);
                        xSemaphoreGive(xMutexDisplay);
                    }
                }
            } else {
                printf("[SAIDA] Nenhum usuario presente.\n");

                if (xSemaphoreTake(xMutexDisplay, pdMS_TO_TICKS(100))) {
                    ssd1306_fill(&ssd, 0);
                    ssd1306_draw_string(&ssd, "VAGAS", 0, 0);
                    ssd1306_draw_string(&ssd, "DISPONIVEIS!", 0, 10);
                    sprintf(buffer, "Usuarios: %d", usuariosAtivos);
                    ssd1306_draw_string(&ssd, buffer, 0, 34);
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

#endif