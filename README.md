# Tarefa 05 – Transmissão de Dados via LoRa

Projeto da disciplina Embarcatech – FPGA

## 📡 Descrição do Projeto

Este projeto implementa um sistema de comunicação sem fio entre um SoC customizado rodando em uma FPGA ColorLight i9 e uma BitDogLab, utilizando módulos LoRa RFM96.

A FPGA atua como nó transmissor, lendo dados ambientais (temperatura e umidade) de um sensor AHT10 (I2C) e enviando-os periodicamente via LoRa (SPI).

A BitDogLab atua como nó receptor, recebendo os dados LoRa e exibindo as medições em um display OLED (I2C).

O projeto integra tanto o desenvolvimento de hardware (SoC LiteX) quanto o desenvolvimento de software (firmware bare-metal para o processador VexRiscv e código embarcado para a BitDogLab).

## ⚙️ Estrutura do Projeto
```bash
Tarefa-05-Embarcatech-FPGA/
│
├── fpga/                     # Arquivos relacionados ao SoC na ColorLight i9
│   ├── build/                # Bitstream e arquivos gerados pelo LiteX
│   └── firmware/             # Código C bare-metal para o VexRiscv
│   └── litex/                # Código Python descrevendo o SoC com oLitex
│
├── bitdoglab/                # Firmware da BitDogLab
│   ├── src/                  # Código-fonte principal
│   └── lib/                  # Bibliotecas LoRa e display
│
└── README.md                 # Este arquivo
```

## 🧠 Funcionamento do Sistema
FPGA (ColorLight i9)

Função: Nó transmissor LoRa.
Componentes principais:

- Core: VexRiscv

- SPI: Interface com o módulo LoRa RFM96 (IDC: CN2)

- I2C: Interface com o sensor AHT10 (JST: J2)

Fluxo de operação:

- Inicializa periféricos SPI e I2C.

- Lê temperatura e a envia pelo rfm96.

BitDogLab

Função: Nó receptor LoRa.
Componentes principais:

- LoRa RFM96 (SPI)

- Display OLED SSD1306 (I2C)

Fluxo de operação:

- Inicializa o módulo LoRa e o display.

- Recebe pacotes enviados pela FPGA.

- Extrai temperatura e umidade.

- Exibe as medições no OLED, atualizando conforme novas transmissões.

## 🔌 Conexões e Pinos Utilizados
### FPGA (ColorLight i9)

- Conector IDC (CN2) → LoRa (SPI)

 - MOSI, MISO, SCK, NSS, GND, 3V3

- Conector JST (J2) → Sensor AHT10

 - SDA, SCL, GND, 3V3

BitDogLab

- IDC Central → Módulo LoRa (SPI)

- I2C → Display OLED SSD1306


## Diagrama de Blocos
![Diagrama de blocos](./img/image.png)


## Instruções de Compilação e Execução

### 1. Preparação do Ambiente

Antes de compilar, certifique-se de ter configurado o ambiente da **OSS CAD Suite**:

```bash
source caminhp/para/o/oss-cad-suite/environment
```

### 2. Geração do SoC com LiteX

Vá para o diretório fpga:


```bash
cd fpga
```


Execute o seguinte comando para gerar o bitsteam do SoC a partir da colorlight_i5 mais o módulo dot_product:

```bash
python3 litex/colorlight_i5.py --board i9 --revision 7.2 --build --cpu-type=picorv32 --ecppack-compress
```

### 3. Compilação do Firmware

Para compilar o firmware que será executado na placa execute os seguintes comando:
```bash
cd firmware/
make
cd ..
```

### 4. Gravação do Bitstream na FPGA

Com isso, grave o bitsteam na placa usando o programa openFPGALoader: 
```bash
openFPGALoader -b colorlight-i5 build/colorlight_i5/gateware/colorlight_i5.bit
```
### 5. Execução e Teste via Terminal Serial
Por fim execute o seguinte comando para carregar o firmware e entrar no terminal da bios:

```bash
litex_term /dev/ttyACM0 --speed 115200 --kernel firmware/main.bin
```
 Após isso, pressione `enter` digite `reboot`.

 Para executar a captura dos dados do sensor aht10 e o envio através do módulo LoRa, execute no prompt do firmware:
 ```bash
RUNTIME> aht10
 ```





