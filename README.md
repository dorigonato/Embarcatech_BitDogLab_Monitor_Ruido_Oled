# Monitor de Ruído com Raspberry Pi Pico W

Este projeto implementa um **monitor inteligente de ruído** utilizando o **Raspberry Pi Pico W** e um **microfone analógico**. Ele exibe os níveis de ruído em um **display OLED**, controla uma **matriz de LEDs RGB WS2812** e envia os dados para um **servidor HTTP** acessível pelo navegador.

## 🚀 **Funcionalidades**
- 📡 **Servidor HTTP via Wi-Fi**: Exibe os valores de ruído no navegador.
- 🎛️ **Controle de Sensibilidade**: Ajusta o limiar de detecção de som com botões e joystick.
- 🌈 **Matriz de LEDs RGB WS2812**: Indica visualmente os níveis de ruído.
- 🔊 **Alerta Sonoro**: Dois buzzers sinalizam quando o som ultrapassa o limite.

---

## 🛠️ **Configuração do Hardware**
| Componente | Pino GPIO |
|------------|----------|
| Microfone (ADC) | GPIO28 |
| Matriz de LEDs WS2812 | GPIO7 |
| Buzzer 1 | GPIO10 |
| Buzzer 2 | GPIO21 |
| Botão A | GPIO5 |
| Botão B | GPIO6 |
| OLED (I2C) SDA | GPIO14 |
| OLED (I2C) SCL | GPIO15 |
| Joystick Vertical (VRY) | GPIO26 |
| Joystick Horizontal (VRX) | GPIO27 |
| Botão do Joystick | GPIO22 |
