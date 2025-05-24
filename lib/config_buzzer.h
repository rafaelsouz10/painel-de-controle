#ifndef CONFIG_BUZZER_H
#define CONFIG_BUZZER_H

#include "hardware/structs/timer.h" // Para usar alarmes via add_alarm_in_us

#define BUZZER 21

// Variáveis de controle
volatile bool buzzer_estado = false; // Controla do estado atual do buzzer (ligado ou desligado)
alarm_id_t buzzer_alarm_id = -1;    // Armazena o ID do alarm ativo (usado para cancelar depois)

void buzzer_init() {
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_put(BUZZER, 0);
}

// Callback do alarme: alterna o buzzer a cada 2000us (simula ~500Hz)
int64_t buzzer_alarm_callback(alarm_id_t id, void *user_data) {
    buzzer_estado = !buzzer_estado;
    gpio_put(BUZZER, buzzer_estado);
    return 2000; // Reexecuta o callback a cada 2ms (500Hz)
}

// Inicia o efeito sonoro do buzzer (se não estiver tocando).
void buzzer_start_alarm() {
    if (buzzer_alarm_id < 0) {  // Só agenda se não tiver um alarm ativo
        buzzer_alarm_id = add_alarm_in_us(2000, buzzer_alarm_callback, NULL, true);
    }
}

// Para o efeito sonoro do buzzer (se estiver tocando).
void buzzer_stop_alarm() {
    if (buzzer_alarm_id >= 0) {         // Só cancela se um alarm estiver ativo
        cancel_alarm(buzzer_alarm_id); // Cancela o alarme
        buzzer_alarm_id = -1;         // Reseta o ID
        gpio_put(BUZZER, 0);         // Desliga fisicamente o buzzer
        buzzer_estado = false;      // Reseta o estado de controle
    }
}

#endif