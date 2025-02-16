# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Ativa suporte ao Wi-Fi no Raspberry Pi Pico W
set(PICO_CYW43_SUPPORTED 1)
set(PICO_CYW43_LWIP 1)  # Ativa a pilha lwIP

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Monitor_Ruido_Oled C CXX ASM)

# Inicializa o SDK do Pico
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Monitor_Ruido_Oled 
                Monitor_Ruido_Oled.c
                ssd1306_i2c.c  # Adiciona o arquivo da biblioteca SSD1306 
                )

# Define o nome e versão do programa
pico_set_program_name(Monitor_Ruido_Oled "Monitor_Ruido_Oled")
pico_set_program_version(Monitor_Ruido_Oled "0.1")

# Modifica para ativar/desativar saída via UART/USB
pico_enable_stdio_uart(Monitor_Ruido_Oled 0)
pico_enable_stdio_usb(Monitor_Ruido_Oled 1)

# Adiciona as bibliotecas necessárias
target_link_libraries(Monitor_Ruido_Oled
        pico_stdlib
        hardware_adc    # Biblioteca para ADC (microfone)
        hardware_pwm    # Biblioteca para controle de PWM (buzzers)
        hardware_pio    # Biblioteca para comunicação com a matriz de LEDs WS2812
        hardware_i2c    # Biblioteca I2C para o display OLED
        pico_cyw43_arch_lwip_threadsafe_background  # Wi-Fi + lwIP
        pico_lwip       # Biblioteca lwIP para rede TCP/IP
        )

# Inclui diretórios necessários
target_include_directories(Monitor_Ruido_Oled PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lwip_config  # Diretório do lwipopts.h
)

# Adiciona os arquivos de saída (UF2, ELF, BIN)

pico_add_extra_outputs(Monitor_Ruido_Oled)

