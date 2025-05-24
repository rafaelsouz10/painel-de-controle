#ifndef TASK_SAIDA_H
#define TASK_SAIDA_H

#define BOTAO_SAIDA 5
#define BOTAO_RESET 22

void vTaskSaida(void *params) {
    // Inicializa o botão de saída com pull-up
    gpio_init(BOTAO_SAIDA);
    gpio_set_dir(BOTAO_SAIDA, GPIO_IN);
    gpio_pull_up(BOTAO_SAIDA);

    bool botaoAnterior = true;   // Variável usada para detectar borda de descida
    char buffer[32];            // Buffer para montar mensagens no display

    while (true) {
        bool botaoAtual = gpio_get(BOTAO_SAIDA);  // Lê o estado atual do botão

        // Detecta borda de descida (botão pressionado)
        if (!botaoAtual && botaoAnterior) {
            if (usuariosAtivos > 0) {
                // Reduz o número de usuários e atualiza semáforo
                if (xSemaphoreTake(xSemaforoContagem, pdMS_TO_TICKS(100)) == pdTRUE) {
                    usuariosAtivos--;
                    atualizarLedRGB(usuariosAtivos);

                    printf("[SAIDA] Usuario saiu. Total: %d\n", usuariosAtivos);  // Mensagem no terminal

                    // Atualiza o display com a saída
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
                // Nenhum usuário presente no sistema
                printf("[SAIDA] Nenhum usuario presente.\n");

                // Atualiza display para indicar que há vagas disponíveis
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
            vTaskDelay(pdMS_TO_TICKS(300));  // Anti-rebounce para evitar múltiplos disparos
        }
        botaoAnterior = botaoAtual;      // Atualiza o estado anterior do botão
        vTaskDelay(pdMS_TO_TICKS(10));  // Delay curto para polling
    }
}

#endif