#ifndef TASK_ENTRADA_H
#define TAKS_ENTRADA_H

#define BOTAO_ENTRADA 6

//TASK DE ENTRADA
void vTaskEntrada(void *params) {
    // Inicializa o botão de entrada com pull-up
    gpio_init(BOTAO_ENTRADA);
    gpio_set_dir(BOTAO_ENTRADA, GPIO_IN);
    gpio_pull_up(BOTAO_ENTRADA);

    bool botaoAnterior = true;  // Armazena o estado anterior do botão para detectar borda de descida
    char buffer[32];           // Buffer para montar as mensagens do display

    while (true) {
        bool botaoAtual = gpio_get(BOTAO_ENTRADA);  // Lê o estado atual do botão

        // Detecta borda de descida (botão pressionado)
        if (!botaoAtual && botaoAnterior) {
            // Verifica se ainda há vagas disponíveis
            if (uxSemaphoreGetCount(xSemaforoContagem) < MAX_USUARIOS) {
                xSemaphoreGive(xSemaforoContagem);  // Libera uma vaga no semáforo
                usuariosAtivos++;                  // Incrementa o número de usuários ativos
                atualizarLedRGB(usuariosAtivos);  // Atualiza o LED RGB com base na ocupação

                printf("[ENTRADA] Usuario entrou. Total: %d\n", usuariosAtivos);  // Feedback no terminal

                // Atualiza o display OLED com mensagem de entrada autorizada
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
                // Caso o sistema esteja lotado, emite um beep curto
                buzzer_start_alarm();
                vTaskDelay(pdMS_TO_TICKS(200));
                buzzer_stop_alarm();

                printf("[ENTRADA] Capacidade maxima atingida (%d)\n", MAX_USUARIOS);  // Mensagem no terminal

                // Atualiza o display OLED com aviso de lotação
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
            vTaskDelay(pdMS_TO_TICKS(300)); // Anti-rebounce: evita múltiplas leituras em um clique
        }
        botaoAnterior = botaoAtual;       // Atualiza o estado anterior do botão
        vTaskDelay(pdMS_TO_TICKS(10));   // Delay curto para reduzir uso da CPU
    }
}

#endif