# GP2040-CE v0.7.12 — MiSTer Cabinet Custom

Firmware personalizado baseado no **GP2040-CE v0.7.12** para **Waveshare RP2040-Zero**, criado para utilização numa máquina arcade com **MiSTer FPGA** e **Zaparoo**.

Ficheiro de firmware:

`GP2040-CE_v0.7.12_WaveshareZero_MiSTerCabinet-Custom.uf2`

## Configuração do cabinet

O controlador fixo está em **Keyboard Mode** e apresenta-se ao MiSTer com o identificador USB **VID:PID `CAFE:0001`** (`VID=0xCAFE`, `PID=0x0001`). Este par identifica este tipo de controlador USB no MiSTer.

O tratamento JAMMA deve manter-se ativo:

```ini
jamma_vid=0xCAFE
jamma_pid=0x0001
```

Isto mantém os botões fixos Coin/Start a funcionar corretamente nos jogos.

GPIO usado pelo botão multifunções:

`GP4 -> A1`

## Porque são usadas F13-F17

O modo JAMMA do MiSTer converte algumas teclas normais antes de estas chegarem ao core. Isso inclui `P`, `Left Ctrl`, `Left Alt` e `Left Shift`.

Por isso o firmware usa teclas que o tratamento JAMMA não converte:

```text
single -> F13
double -> F12
long   -> F14 + F15 + F16 + F17
```

É também **obrigatório** copiar para o MiSTer o ficheiro binário:

`/media/fat/config/kbd_cafe_0001.map`

Esse mapa é específico do dispositivo `CAFE:0001` e faz:

```text
F13 -> P
F14 -> Left Shift
F15 -> Left Ctrl
F16 -> Left Alt
F17 -> Right Alt
```

Assim, Coin/Start continuam a beneficiar do modo JAMMA, enquanto o botão multifunções volta a comportar-se como teclado normal depois dessa conversão.

## Instalação do mapa obrigatório

O ZIP `GP2040-CE-MiSTer-Cabinet-source-and-firmware.zip` inclui:

- `firmware/`: firmware UF2 para a Waveshare RP2040-Zero.
- `config/kbd_cafe_0001.map`: mapa binário pronto a instalar (256 bytes).
- `source/`: código-fonte, gerador e testes.
- `BUILD_INFO.txt`: commit e SHA256 do firmware e do mapa.

Depois de extrair o ZIP, copiar **apenas** `config/kbd_cafe_0001.map` para
`/media/fat/config/kbd_cafe_0001.map` no cartão do MiSTer. Se já existir um
ficheiro com esse nome, guardar primeiro uma cópia de segurança: esta instalação
substitui os remapeamentos de teclado desse dispositivo. Não substituir a pasta
`config` inteira. Manter `jamma_vid=0xCAFE`, `jamma_pid=0x0001`,
`input_cafe_0001_v3.map` e todos os mapas por jogo existentes.

Comparar o SHA256 do ficheiro copiado (`sha256sum /media/fat/config/kbd_cafe_0001.map`)
com `Keyboard map SHA256` em `BUILD_INFO.txt`. Reiniciar o MiSTer para recarregar
o mapa de teclado. O duplo clique continua a enviar F12 sem remapeamento.
Para reverter, restaurar a cópia anterior (ou remover apenas este mapa se antes
não existia) e reiniciar; sem este mapa o single/long custom deixa de funcionar.

### Geração reproduzível e formato

Na raiz do código-fonte, com Python 3, sem dependências externas:

```sh
python3 scripts/generate_mister_kbd_map.py config/kbd_cafe_0001.map
python3 tests/test_mister_kbd_map.py
```

O gerador escreve exatamente 256 bytes, sem cabeçalho: uma tabela `uint8_t[256]`
indexada pelo código **Linux EV_KEY**, não pelo USB HID usage emitido pelo firmware.
Cada byte é o código Linux de destino; zero mantém o código original. Como cada
entrada tem um byte, não há dependência de endianness. Só cinco bytes são não nulos:

| Origem | Índice Linux (decimal / hex) | Destino | Byte (decimal / hex) |
|---|---|---|---|
| F13 | 183 / B7 | P | 25 / 19 |
| F14 | 184 / B8 | Left Shift | 42 / 2A |
| F15 | 185 / B9 | Left Ctrl | 29 / 1D |
| F16 | 186 / BA | Left Alt | 56 / 38 |
| F17 | 187 / BB | Right Alt | 100 / 64 |

Formato confirmado em [Main_MiSTer `input.cpp`](https://github.com/MiSTer-devel/Main_MiSTer/blob/63186a34544fccb3c0755856a55670d1a0a8daf9/input.cpp):
`kbdmap[256]`, `get_kbdmap_name`, `finish_map_setting` e `input_cb`.
[`FileSaveConfig` / `FileLoadConfig`](https://github.com/MiSTer-devel/Main_MiSTer/blob/63186a34544fccb3c0755856a55670d1a0a8daf9/file_io.cpp)
guardam/carregam os bytes diretamente. Os códigos estão definidos na ABI Linux
[`input-event-codes.h`](https://github.com/torvalds/linux/blob/v6.12/include/uapi/linux/input-event-codes.h).

No processamento do MiSTer, `QUIRK_JAMMA` converte Coin/Start em códigos de joystick
antes de chamar `input_cb`. F13–F17 não constam dessa tabela JAMMA e chegam ao
remapeamento de teclado, aplicado uma única vez a códigos inferiores a 256.
Por isso P e os modificadores resultantes não voltam a passar pela conversão JAMMA.
Os eventos de libertação usam o mesmo mapa; o valor press/release/repeat é preservado.

O teste offline compara os 256 bytes com um resultado independente, verifica a
reprodutibilidade e as teclas configuradas em `BoardConfig.h`, e cobre press/release,
F12 e a passagem dos códigos Coin/Start. A CI também verifica o mapa e o SHA256
**dentro do ZIP** antes do upload. Isto valida o contrato binário e a consistência
com o firmware; não substitui testes de USB/GPIO nem confirma a versão/configuração
do MiSTer instalado no cabinet.

## Funções do botão

- **Um clique curto (até 500 ms):** aguarda **300 ms** para excluir duplo clique e envia `F13` durante cerca de **200 ms**. No MiSTer, `F13` é convertido em `P` para Pause.
- **Duplo clique:** exige duas pressões curtas (cada uma até **500 ms**), com a segunda a começar dentro da janela de **300 ms**. Envia `F12` durante cerca de **80 ms** ao largar o segundo toque curto. Abre/fecha o OSD.
- **Pressão intermédia (mais de 500 ms e menos de 2 segundos):** não executa qualquer ação.
- **Pressão longa (2 segundos ou mais):** envia `F14 + F15 + F16 + F17` durante cerca de **500 ms**. No MiSTer são convertidas em `Left Shift + Left Ctrl + Left Alt + Right Alt`, para abandonar o jogo e regressar ao Zaparoo Frontend.
- A pressão longa dispara uma única vez.
- Depois de uma pressão longa, largar o botão não envia outra ação.
- Primeiro clique curto + segundo toque mantido por 2 segundos ou mais: é enviada apenas a ação longa.
- O input **A1 tem debounce por software de 30 ms**.
- A janela de duplo clique conta desde a libertação confirmada pelo debounce do primeiro toque até ao início da pressão física estável do segundo. O intervalo tem de ser **inferior a 300 ms**: aos 300 ms a janela já expirou. Um toque iniciado antes do limite continua válido se o debounce de 30 ms terminar depois; bounce reinicia a estabilização e não prolonga a janela.

## Botões fixos Coin/Start

Estes não mudam:

```text
S1 -> 5 -> Coin 1
S2 -> 1 -> Start P1
L3 -> 2 -> Start P2
R3 -> 6 -> Coin 2
```

O tratamento JAMMA `CAFE:0001` e os atuais ficheiros `input_cafe_0001_v3.map` / mapas por jogo podem permanecer como estão.

## Importante

O comportamento single/double/long está implementado no firmware e não aparece no Web Configurator.

O valor atribuído a `A1` na página **Keyboard Mapping** não é usado por esta função custom, porque o firmware interceta diretamente o input lógico `A1`.

O botão físico de **RESET** continua separado desta máquina de estados.

## Base

- GP2040-CE: **v0.7.12**
- Board: **Waveshare RP2040-Zero**
- Modo: **Keyboard**
- USB: **CAFE:0001**
- Produto USB: **GP2040-CE - Fixed Arcade Buttons**
- Janela de duplo clique: **300 ms**
- Pressão curta máxima: **500 ms**
- Debounce A1: **30 ms**
- Pressão longa: **2000 ms**
- Impulso single (`F13`): **200 ms**
- Impulso double (`F12`): **80 ms**
- Impulso long (`F14-F17`): **500 ms**

## Testes de regressão

Com Python 3 e um compilador C++17 no host, executar na raiz do repositório:

```sh
python3 tests/run_mister_menu_tests.py
```

A suite compila diretamente o bloco A1 de `KeyboardDriver.cpp`, os seus campos
de estado e métodos de atalhos, usando as constantes de `WaveshareZero/BoardConfig.h`.
Apenas o relógio, o input e a recolha das teclas são simulados; não há uma cópia da
máquina de estados nos testes. Cobre os limites de 299/300/301 ms, debounce a
atravessar o prazo, bounce, pausas no processamento, pressões de 500/501 e
1999/2000 ms, duração dos impulsos, disparo único da pressão longa e rollover do
relógio de 32 bits. O workflow `MiSTer menu regression tests` executa a suite em CI.
Estes testes não validam GPIO, transmissão USB ou comportamento físico no MiSTer.
