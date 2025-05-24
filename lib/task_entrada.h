#ifndef TASK_ENTRADA_H
#define TAKS_ENTRADA_H

#define BOTAO_ENTRADA 6

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
                    ssd1306_draw_string(&ssd, "ENTRADA", 0, 0);
                    ssd1306_draw_string(&ssd, "AUTORIZADA", 0, 10);
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
                    ssd1306_draw_string(&ssd, "LOTADO!", 0, 0);
                    ssd1306_draw_string(&ssd, "AGUARDE...", 0, 16);
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