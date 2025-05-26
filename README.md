# Painel Interativo com Controle de Acesso Multitarefas

---

## Descrição do Projeto
Sistema embarcado de controle de acesso com limite de ocupação, utilizando a placa BitDogLab e o sistema operacional em tempo real FreeRTOS. O sistema deve oferecer sinalizações visual e sonora sobre o estado de ocupação, permitindo entrada, saída e reinicialização segura do contador de usuários.

---

## Requisitos

- **Microcontrolador**: Raspberry Pi Pico ou Raspberry Pi Pico W (opcional)
- **Placa de Desenvolvimento:** BitDogLab (opcional).
- **Editor de Código**: Visual Studio Code (VS Code).
- **SDK do Raspberry Pi Pico** configurado no sistema.
- Ferramentas de build: **CMake** e **Ninja**.

---

## Instruções de Uso

### 1. Clone o Repositório

Clone o repositório para o seu computador:
```bash
git clone https://github.com/rafaelsouz10/painel-de-controle.git
cd painel-de-controle
code .
```
---

### 2. Instale as Dependências

### 2.1 Certifique-se de que o SDK do Raspberry Pi Pico está configurado corretamente no VS Code. As extensões recomendadas são:

- **C/C++** (Microsoft).
- **CMake Tools**.
- **Raspberry Pi Pico**.

### 2.1 Configure o FreeRTOS (manual)

Para utilizar o FreeRTOS com a Raspberry Pi Pico, você deve fazer a integração manual do kernel ao seu projeto.

**Passo a passo:**

1. Baixe o kernel do FreeRTOS (ou clone do GitHub):
   ```bash
   git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git

2. Defina o caminho do kernel no seu CMakeLists.txt na linha 31:

    ```CMakeLists.txt
    set(FREERTOS_KERNEL_PATH "E:/CAMINHO-DO-KERNEL-BAIXADO/FreeRTOS-Kernel")

---

### 3. Configure o VS Code

Abra o projeto no Visual Studio Code e siga as etapas abaixo:

1. Certifique-se de que as extensões mencionadas anteriormente estão instaladas.
2. No terminal do VS Code, compile o código clicando em "Compile Project" na interface da extensão do Raspberry Pi Pico.
3. O processo gerará o arquivo `.uf2` necessário para a execução no hardware real.

---

### 4. Teste no Hardware Real

#### Utilizando a Placa de Desenvolvimento BitDogLab com Raspberry Pi Pico W:

1. Conecte a placa ao computador no modo BOTSEEL:
   - Pressione o botão **BOOTSEL** (localizado na parte de trás da placa de desenvolvimento) e, em seguida, o botão **RESET** (localizado na frente da placa).
   - Após alguns segundos, solte o botão **RESET** e, logo depois, solte o botão **BOOTSEL**.
   - A placa entrará no modo de programação.

2. Compile o projeto no VS Code utilizando a extensão do [Raspberry Pi Pico W](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico):
   - Clique em **Compile Project**.

3. Execute o projeto clicando em **Run Project USB**, localizado abaixo do botão **Compile Project**.

---

### 🔌 5. Conexões e Esquemático
Abaixo está o mapeamento de conexões entre os componentes e a Raspberry Pi Pico W:

| **Componentes**        | **Pino Conectado (GPIO)** |
|------------------------|---------------------------|
| Display SSD1306 (SDA)  | GPIO 14                   |
| Display SSD1306 (SCL)  | GPIO 15                   |
| Botão do Joystick      | GPIO 22                   |
| LED RGB Vermelho       | GPIO 13                   |
| LED RGB Azul           | GPIO 12                   |
| LED RGB Verde          | GPIO 11                   |
| Buzzer                 | GPIO 21                   |
| Botão A                | GPIO 5                    |
| Botão B                | GPIO 6                    |

#### 🛠️ Hardware Utilizado
- **Microcontrolador Raspberry Pi Pico W**
- **Display OLED SSD1306 (I2C)**
- **Botão A e B**
- **Buzzer**
- **LED RGB**
- **FreeRTOS**
- **Joystick**

---

### 6. Funcionamento do Sistema

#### 📌 Funcionalidades

O sistema desenvolvido simula o **controle de acesso de usuários** a um **ambiente físico com capacidade limitada**, como **laboratórios** ou **bibliotecas**. Ele é baseado em **três tarefas multitarefa** rodando em paralelo com **FreeRTOS**:

- **Entrada de Usuário (Botão A):**  
  Ao ser pressionado, verifica se ainda há **vagas disponíveis**. Se sim, incrementa a variável de ocupação (**usuariosAtivos**), **atualiza a cor do LED RGB** e **exibe uma mensagem de “Entrada autorizada” no display OLED**.  
  Se a sala estiver lotada, **emite um beep** e exibe **“Lotado! Aguarde...”**.

- **Saída de Usuário (Botão B):**  
  Quando pressionado, verifica se há **usuários presentes**. Se houver, **decrementa o contador de ocupação**, **libera uma vaga no semáforo de contagem**, e **atualiza o LED** e o **display com a mensagem “Saída realizada”**.  
  Se não houver ninguém, **exibe “Vagas disponíveis!”**.

- **Reset do Sistema (Botão do Joystick):**  
  Acionado por **interrupção**, **reseta todo o sistema**: **zera o número de usuários**, **recria o semáforo de contagem**, **emite dois beeps curtos** e **exibe “Sistema resetado” no display**.

O sistema fornece **feedback em tempo real** via **display OLED**, **LED RGB** e **buzzer**, garantindo **operação segura entre múltiplas tarefas** com **sincronização adequada**.

---

### 7. Vídeos Demonstrativo

**Click [AQUI](https://drive.google.com/file/d/154iCgYJpqPHi0RqOQWJ-RA5x3dlu2Ouo/view?usp=sharing) para acessar o link do Vídeo Ensaio**