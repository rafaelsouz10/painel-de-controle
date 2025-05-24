#ifndef CONFIG_LEDS_H
#define CONFIG_LEDS_H

// Definição dos pinos dos LEDs
#define LED_R 13  
#define LED_B 12
#define LED_G 11

void leds_init(){
    //Configura os led como saída
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
}

// Função responsável para acionar os leds a partir da quantidade de usuários
void atualizarLedRGB(uint8_t usuarios) {
    if (usuarios == 0) {
        gpio_put(LED_G, 0); gpio_put(LED_R, 0); gpio_put(LED_B, 1); 
    } else if (usuarios > 0 && usuarios < MAX_USUARIOS) {
        gpio_put(LED_B, 0); gpio_put(LED_R, 0); gpio_put(LED_G, 1);
    } else {
        gpio_put(LED_B, 0); gpio_put(LED_G, 0); gpio_put(LED_R, 1);
    }
}

#endif 