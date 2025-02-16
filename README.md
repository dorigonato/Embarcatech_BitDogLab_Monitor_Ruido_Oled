# 🎛️ Monitor de Ruído com Raspberry Pi Pico W e BitDogLab

Este projeto implementa um **monitor inteligente de ruído** utilizando o **Raspberry Pi Pico W** e um **microfone analógico**. Ele exibe os níveis de ruído em um **display OLED**, controla uma **matriz de LEDs RGB WS2812** e envia os dados para um **servidor HTTP**, permitindo monitoramento via navegador.

---

## 🚀 Funcionalidades
- **📡 Servidor HTTP via Wi-Fi**: Exibe os valores de ruído no navegador.
- **🎛️ Ajuste de Sensibilidade**: Controle do limiar de detecção via botões e joystick.
- **🌈 Matriz de LEDs RGB WS2812**: Indica visualmente os níveis de ruído.
- **🔊 Alerta Sonoro**: Dois buzzers ativados quando o som ultrapassa o limite.

---

## 🛠️ Hardware Utilizado
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

---

## 📜 Funções Implementadas

### 🔹 Wi-Fi e Servidor HTTP
- `void start_http_server();` → Inicia o servidor HTTP e define callbacks.
- `static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);` → Processa requisições HTTP.
- `void generate_http_response(uint16_t valorADC);` → Gera resposta HTTP com valores do sensor.

### 🔹 Leitura do Microfone
- `void configurarADC();` → Configura o ADC para leitura do microfone.
- `uint16_t lerSom();` → Retorna o nível de ruído (0-4095).

### 🔹 Controle de LEDs
- `void configurarMatrizLEDs();` → Configura matriz de LEDs WS2812.
- `void setPixelColor(int index, uint8_t r, uint8_t g, uint8_t b);` → Define cor de um LED.
- `void fillMatrix(uint8_t r, uint8_t g, uint8_t b);` → Preenche matriz com uma cor.

### 🔹 Controle de Buzzers
- `void configurarPWM();` → Configura PWM para buzzers.
- `void setBuzzerFrequency(uint frequency);` → Ajusta frequência do som emitido.

### 🔹 Controles e Joystick
- `void configurarBotoes();` → Configura botões A e B.
- `bool botaoPressionadoA();` / `bool botaoPressionadoB();` → Retorna `true` se pressionado.
- `uint16_t lerJoystickY();` → Lê valor do eixo Y do joystick.

### 🔹 Exibição no OLED
- `void configurarOLED();` → Configura display OLED via I2C.
- `void atualizarOLED(uint16_t valorADC);` → Atualiza valores exibidos no OLED.

---

## 🖥️ Como Compilar e Rodar
### **1️⃣ Instalar Dependências**
- **Raspberry Pi Pico SDK**
- **CMake**
- **Toolchain ARM GCC**

### **2️⃣ Compilar o Código**
```sh
cd build
cmake ..
make
```

### **3️⃣ Subir para o Raspberry**
```sh
cp Monitor_Ruido_Oled.uf2 /media/pi/RPI-RP2/
```

---

## 🌐 Como Acessar no Navegador
1️⃣ Conecte-se à mesma rede Wi-Fi do Raspberry.
2️⃣ Descubra o **IP do Raspberry** (exibido no Serial Monitor).
3️⃣ Acesse via navegador:
   ```sh
   http://<IP_DO_RASPBERRY>
   ```
   Exemplo:
   ```sh
   http://192.168.100.84
   ```

---

## 📢 Contato e Contribuição
🔹 **Autor:** _Dorival Rigonato Júnior_
🔹 **Contribuições:** Pull requests são bem-vindos!  

---

🚀 **Projeto publicado e funcional no GitHub!**

