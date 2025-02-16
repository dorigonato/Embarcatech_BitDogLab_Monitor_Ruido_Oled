#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include <string.h>
#include <ctype.h>
#include "pico/binary_info.h"
#include "ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"

// Definição dos pinos utilizados
#define MIC_ADC 28  // Microfone
#define LED1 11  // LED no GPIO11
#define LED2 12  // LED no GPIO12
#define LED3 13  // LED no GPIO13
#define BOTAO_A 5  // Botão A no GPIO5
#define BOTAO_B 6  // Botão B no GPIO6
#define I2C_SDA 14 // Pino SDA do OLED
#define I2C_SCL 15 // Pino SCL do OLED
#define LED_MATRIZ 7 // Matriz de LEDs WS2812
#define NUM_LEDS 25  // Total de LEDs na matriz
#define BUZZER1 10 // Buzzer 1 no GPIO10
#define BUZZER2 21 // Buzzer 2 no GPIO21
#define VRX 27 // Joystick eixo X
#define VRY 26 // Joystick eixo Y
#define SW 22  // Botão do Joystick
#define LED_PICO 25 // Led da placa do Raspberry Pi Pico

// Definição das credenciais de Wi-Fi
#define WIFI_SSID "escrever aqui o SSID"  // Substitua pelo nome da rede Wi-Fi
#define WIFI_PASS "escrever aqui a senha da rede wi-fi" // Substitua pela senha da rede Wi-Fi

// Variáveis globais
PIO pio;
int sm;
uint32_t leds[NUM_LEDS]; // Estado da matriz de LEDs
int LIMIAR_SILENCIO = 2095;  // Valor padrão do limiar do microfone
bool buzzersAtivos = true; // Estado dos buzzers
char http_data_response[1024]; // Buffer para resposta HTTP

// Função para configurar o LED embutido na placa do Raspberry Pi Pico
void configurarLEDPico() {
    gpio_init(LED_PICO);
    gpio_set_dir(LED_PICO, GPIO_OUT);
}

// Configura o PWM para controlar a frequência do buzzer
void configurarPWM() {
    gpio_set_function(BUZZER1, GPIO_FUNC_PWM);
    gpio_set_function(BUZZER2, GPIO_FUNC_PWM);

    uint slice_num1 = pwm_gpio_to_slice_num(BUZZER1);
    uint slice_num2 = pwm_gpio_to_slice_num(BUZZER2);

    pwm_set_wrap(slice_num1, 12500);
    pwm_set_wrap(slice_num2, 12500);
    pwm_set_clkdiv(slice_num1, 64.0);
    pwm_set_clkdiv(slice_num2, 64.0);

    pwm_set_enabled(slice_num1, true);
    pwm_set_enabled(slice_num2, true);
}

// Ajusta a frequência do som emitido pelos buzzers
void setBuzzerFrequency(uint frequency) {
    if (buzzersAtivos && frequency > 0) {
        uint slice_num1 = pwm_gpio_to_slice_num(BUZZER1);
        uint slice_num2 = pwm_gpio_to_slice_num(BUZZER2);

        uint wrap_value = 125000000 / (64 * frequency);

        pwm_set_wrap(slice_num1, wrap_value);
        pwm_set_wrap(slice_num2, wrap_value);

        pwm_set_gpio_level(BUZZER1, wrap_value / 2);
        pwm_set_gpio_level(BUZZER2, wrap_value / 2);
    } else {
        pwm_set_gpio_level(BUZZER1, 0);
        pwm_set_gpio_level(BUZZER2, 0);
    }
}

// Função para verificar o botão do joystick
void configurarBotaoJoystick() {
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);
}

// Verifica se o botão do joystick foi pressionado
bool botaoJoystickPressionado() {
    if (gpio_get(SW) == 0) {
        sleep_ms(100);
        if (gpio_get(SW) == 0) {
            return true;
        }
    }
    return false;
}

// Configuração da Matriz de LEDs WS2812
void configurarMatrizLEDs() {
    pio = pio0;
    sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_MATRIZ, 800000, false);
    
// Função para atualizar a matriz de LEDs

    // Inicializa todos os LEDs apagados
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = 0;
    }
}

// Função para definir a cor de um LED específico na Matriz WS2812
void setPixelColor(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < 0 || index >= NUM_LEDS) return;
    leds[index] = ((uint32_t)(g & 0xFF) << 8) | ((uint32_t)(r & 0xFF) << 16) | (b & 0xFF); // Corrigindo ordem RGB -> GRB
}

// Função para atualizar a matriz de LEDs
void updateMatrix() {
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio, sm, leds[i]);
    }
}

// Função para preencher toda a matriz com uma única cor
void fillMatrix(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = ((uint32_t)g << 8) | ((uint32_t)r << 16) | b; // Formato GRB
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
    updateMatrix();
}

// Configuração dos Buzzers
void configurarBuzzers() {
    gpio_init(BUZZER1);
    gpio_set_dir(BUZZER1, GPIO_OUT);
    gpio_put(BUZZER1, 0);
    gpio_init(BUZZER2);
    gpio_set_dir(BUZZER2, GPIO_OUT);
    gpio_put(BUZZER2, 0);
}

// Função para ativar os buzzers
void ativarBuzzers(int duracao) {
    gpio_put(BUZZER1, 1);
    gpio_put(BUZZER2, 1);
    sleep_ms(duracao);
    gpio_put(BUZZER1, 0);
    gpio_put(BUZZER2, 0);
}

// Configuração do Joystick
void configurarJoystick() {
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);
}

// Função para ler o eixo Y do joystick
uint16_t lerJoystickY() {
    adc_select_input(0); // Seleciona VRY
    return adc_read();
}

// Função para configurar o ADC do microfone
void configurarADC() {
    adc_init();
    adc_gpio_init(MIC_ADC);
    adc_select_input(2);
}

// Função para ler o nível de som do microfone
uint16_t lerSom() {
    adc_select_input(2); // Seleciona MIC_ADC (GPIO28, canal 2)
    return adc_read();  // Lê o valor do ADC (0-4095)
}

// Função para configurar os LEDs
void configurarLEDs() {
    gpio_init(LED1);
    gpio_set_dir(LED1, GPIO_OUT);
    gpio_init(LED2);
    gpio_set_dir(LED2, GPIO_OUT);
    gpio_init(LED3);
    gpio_set_dir(LED3, GPIO_OUT);
}

// Função para configurar os botões
void configurarBotoes() {
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
}

// Função para evitar bounce do botão A
bool botaoPressionadoA() {
    if (gpio_get(BOTAO_A) == 0) {
        sleep_ms(5);
        if (gpio_get(BOTAO_A) == 0) return true;
    }
    return false;
}

// Função para evitar bounce do botão B
bool botaoPressionadoB() {
    if (gpio_get(BOTAO_B) == 0) {
        sleep_ms(5);
        if (gpio_get(BOTAO_B) == 0) return true;
    }
    return false;
}

// Configuração do OLED
void configurarOLED() {
    i2c_init(i2c1, 800 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();
    
    // Preparar área de renderização para o display 
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // Limpa o display
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

// Atualizar mensagem no OLED
void atualizarOLED(uint16_t valorADC) {
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // Limpa o display
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    //escreve mensagem no dislpay OLED
    char text[50]; // Buffer para armazenar a string formatada
    int y = 5;

    snprintf(text, sizeof(text), "Limiar Silencio: %d");
    ssd1306_draw_string(ssd, 2, y, text);  // Escreve a string formatada

    y += 15; // Incrementa a posição Y para a próxima linha

    snprintf(text, sizeof(text), "%d", LIMIAR_SILENCIO);
    ssd1306_draw_string(ssd, 45, y, text);   // Segunda linha

    y += 15; // Incrementa a posição Y para a próxima linha

    snprintf(text, sizeof(text), "Valor Microfone:");
    ssd1306_draw_string(ssd, 2, y, text); //Terceira linha

    y += 15; // Incrementa a posição Y para a próxima linha

    snprintf(text, sizeof(text), "%d", valorADC);
    ssd1306_draw_string(ssd, 45, y, text);

    render_on_display(ssd, &frame_area);

}

// Função para gerar a resposta HTTP com os dados
void generate_http_response(uint16_t valorADC) {
    snprintf(http_data_response, sizeof(http_data_response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"  // Garante que o cliente encerre corretamente a conexão
        "Content-Length: %d\r\n"
        "\r\n"
        "<!DOCTYPE html><html><body>"
        "<h1>Dados do Sensor</h1>"
        "<p><strong>Limiar Silencio:</strong> %d</p>"
        "<p><strong>Valor Microfone:</strong> %d</p>"
        "</body></html>\r\n",
        strlen("<!DOCTYPE html><html><body>"
               "<h1>Dados do Sensor</h1>"
               "<p><strong>Limiar Silencio:</strong> 0000</p>"
               "<p><strong>Valor Microfone:</strong> 0000</p>"
               "</body></html>\r\n"), // Calcula o tamanho real da resposta
        LIMIAR_SILENCIO, valorADC);
}

// Função de callback para processar requisições HTTP
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (err != ERR_OK || p == NULL) {
        printf("Erro na conexão ou cliente desconectou.\n");
        if (p != NULL) pbuf_free(p);
        return ERR_OK;
    }

    // Lê o som do microfone e gera a resposta HTTP
    uint16_t valorADC_local = lerSom();
    generate_http_response(valorADC_local);

    printf("Preparando resposta HTTP...\n");
    printf("Resposta:\n%s\n", http_data_response);

    // Enviar a resposta HTTP
    err_t write_err = tcp_write(tpcb, http_data_response, strlen(http_data_response), TCP_WRITE_FLAG_COPY);
    if (write_err != ERR_OK) {
        printf("Erro ao escrever dados TCP: %d\n", write_err);
        pbuf_free(p);
        return ERR_ABRT;
    }

    printf("Resposta enviada, chamando tcp_output()...\n");

    // Força o envio imediato da resposta
    err_t flush_err = tcp_output(tpcb);
    if (flush_err != ERR_OK) {
        printf("Erro ao enviar dados TCP: %d\n", flush_err);
    } else {
        printf("Resposta HTTP enviada com sucesso!\n");
    }

    // Libera buffer recebido
    pbuf_free(p);

    // Em vez de fechar a conexão aqui, esperar que o cliente a feche primeiro
    printf("Aguardando cliente fechar conexão...\n");
    tcp_recv(tpcb, NULL); // Remove o callback de recepção
    return ERR_OK;
}

// Callback de conexão: associa o http_callback à conexão
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);  // Associa o callback HTTP
    
    return ERR_OK;
}

// Função de setup do servidor TCP
static void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }

    // Liga o servidor na porta 80
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }

    printf("Servidor ligado na porta 80! Aguardando conexões...\n");
    
    pcb = tcp_listen(pcb);  // Coloca o PCB em modo de escuta
    tcp_accept(pcb, connection_callback);  // Associa o callback de conexão

    printf("Servidor HTTP rodando na porta 80...\n");
}

// Loop principal
int main() {
    stdio_init_all();
    sleep_ms(10000); //Adicionei um delay para dar tempo de carregar o monitor serial
    configurarADC(); // Função para microfone
    configurarLEDs(); // Função para controlar os led´s
    configurarBotoes(); // Função para controlar os botões A e B
    configurarOLED(); // Função para controlar o display OLED
    configurarMatrizLEDs(); // Função para controlar Matriz WS2812
    configurarJoystick(); // Adicionando a configuração do joystick
    configurarPWM(); // Configura PWM para controle dos buzzers
    configurarBotaoJoystick(); // Configura botão do joystick
    configurarLEDPico();  // Configuração do LED embutido na placa Raspberry
    
    // Inicializa o Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar o Wi-Fi\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        return 1;
    }else {
        printf("Connected.\n");
        // Ler o endereço IP de uma forma legível
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("Endereço IP %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
        printf("Para visualizar os dados acesse o Endereço IP no navegador\n");

    }

    printf("Wi-Fi conectado!\n");

    // Inicia o servidor HTTP
    start_http_server();

    while (1) {
        // Atualiza o limiar baseado nos botões
        if (botaoPressionadoA()) {
            LIMIAR_SILENCIO += 10;
            printf("Novo limiar: %d\n", LIMIAR_SILENCIO);
            sleep_ms(300);
        }
        if (botaoPressionadoB()) {
            LIMIAR_SILENCIO -= 10;
            printf("Novo limiar: %d\n", LIMIAR_SILENCIO);
            sleep_ms(300);
        }

        // Lê o som do microfone
        uint16_t valorADC = lerSom();

        // Verifica se o botão do joystick foi pressionado para alternar o estado dos buzzers
        if (botaoJoystickPressionado()) {
            buzzersAtivos = !buzzersAtivos;
            printf("Buzzers %s\n", buzzersAtivos ? "Ativados" : "Desativados");
            sleep_ms(150);
        }

        // Ajusta a frequência do buzzer com base no valor do microfone
        if (valorADC > LIMIAR_SILENCIO) {
            uint frequency = 100 + ((valorADC - LIMIAR_SILENCIO) * 5);
            setBuzzerFrequency(frequency);
        } else {
            setBuzzerFrequency(0);
        }

        // Controle do joystick
        uint16_t joystickY = lerJoystickY();
        if (joystickY > 3000) { // Para cima
            LIMIAR_SILENCIO += 1;
            printf("Joystick UP - Novo limiar: %d\n", LIMIAR_SILENCIO);
            sleep_ms(300);
        }
        if (joystickY < 1000) { // Para baixo
            LIMIAR_SILENCIO -= 1;
            printf("Joystick DOWN - Novo limiar: %d\n", LIMIAR_SILENCIO);
            sleep_ms(300);
        }

        // Controla os LEDs
        if (valorADC > LIMIAR_SILENCIO) {
            gpio_put(LED1, 1);
            gpio_put(LED_PICO, 1);
            fillMatrix(120, 0, 0); // Matriz
            sleep_ms(100);
            gpio_put(LED1, 0);
            gpio_put(LED2, 1);
            gpio_put(LED_PICO, 1);
            fillMatrix(0, 100, 0); // Matriz
            sleep_ms(100);
            gpio_put(LED2, 0);
            gpio_put(LED3, 1);
            fillMatrix(100, 100, 100); // Matriz
            sleep_ms(100);
            gpio_put(LED3, 0);
            gpio_put(LED_PICO, 1);
            fillMatrix(255, 255, 255); // Matriz
        } else {
            gpio_put(LED1, 0);
            gpio_put(LED2, 0);
            gpio_put(LED3, 0);
            gpio_put(LED_PICO, 0);
            fillMatrix(0, 0, 0); // Matriz
        }

        // Atualiza a tela do OLED com o limiar
        atualizarOLED(valorADC);

        // Gera a resposta HTTP com os dados atuais
        generate_http_response(valorADC);

        cyw43_arch_poll();  // Necessário para manter o Wi-Fi ativo
        sleep_ms(50);
    }
}