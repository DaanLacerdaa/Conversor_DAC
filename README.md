# **Controle de Joystick com LED RGB e Display SSD1306 no Raspberry Pi Pico (BitDogLab)**

## **Demonstração do Projeto**

🎥 Vídeo de funcionamento:

![Projeto Rodando](https://youtu.be/000000000)

## **Descrição**

Este projeto utiliza um **Raspberry Pi Pico W (BitDogLab)** para capturar os valores analógicos de um **joystick**, controlar a intensidade dos LEDs RGB com **PWM** e exibir um **quadrado móvel** em um display **SSD1306 (I2C)**.

Além disso, há diferentes **estilos de borda** no display, alternáveis pelo botão do joystick. O projeto também implementa **debouncing** e **interrupções** para melhor resposta dos botões.

---

## **Funcionamento**

- O **joystick** controla **dois LEDs**:
  - **LED Azul**: brilho ajustado pelo eixo **Y** do joystick.
  - **LED Vermelho**: brilho ajustado pelo eixo **X** do joystick.
- O **display SSD1306** exibe um **quadrado móvel**, cuja posição muda conforme o movimento do joystick.
- O **botão do joystick** alterna o **LED Verde** e altera o **estilo da borda do display**.
- O **botão A** ativa/desativa os LEDs controlados por PWM.
- Implementação de **interrupções** para os botões, garantindo resposta instantânea.
- **Debouncing via software** para evitar leituras duplicadas ao pressionar os botões.

---

## **Componentes Utilizados**

- **Placa**: Raspberry Pi Pico W (BitDogLab)
- **Joystick** (conectado às GPIOs 26 e 27)
- **LED RGB** (pinos 11 - verde , 12 - azul e 13 - vermelho)
- **Botões**: Joystick Button (GPIO 22) e Button A (GPIO 5)
- **Display OLED SSD1306 (I2C)** (GPIOs 14 e 15)

---

## **Estilos de Borda do Display**

1. **Sem borda**
2. **Borda sólida**
3. **Borda dupla**
4. **Borda arredondada**
5. **Borda pontilhada**

O estilo de borda muda a cada **pressionamento do botão do joystick**.

---

## **Como Executar no Visual Studio Code com Pico SDK**

### **1. Configurar o Raspberry Pi Pico SDK**

Se ainda não configurou o **Pico SDK**, siga estes passos:

1. **Instale o Raspberry Pi Pico SDK** seguindo a documentação oficial:  
   👉 [Guia de Instalação do Pico SDK](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
2. **Instale o Visual Studio Code** e os pacotes:
   - CMake
   - Ninja
   - ARM GCC

3. **Clone este repositório**:

   ```sh
   ``` git clone https://github.com/SEU_GITHUB/Conversor_DAC.git
   ``` cd Conversor_DAC

4. **Compilar e Executar o Código**

Crie a pasta de build e gere os arquivos necessários:

mkdir build
cd build
cmake ..
Compile o projeto:
make

Conecte o Raspberry Pi Pico W ao PC segurando o botão BOOTSEL e envie por meio do Raspberry Pico SDK ou arraste o arquivo .uf2 gerado para o dispositivo montado.

Autor
👤 SEU NOME
