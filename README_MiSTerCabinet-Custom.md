# GP2040-CE v0.7.12 — MiSTer Cabinet Custom

Firmware personalizado baseado no **GP2040-CE v0.7.12** para **Waveshare RP2040-Zero**, criado para utilização numa máquina arcade com **MiSTer FPGA** e **Zaparoo**.

Ficheiro de firmware:

`GP2040-CE_v0.7.12_WaveshareZero_MiSTerCabinet-Custom.uf2`

## Configuração do cabinet

O controlador fixo está em **Keyboard Mode** e o MiSTer mantém o tratamento JAMMA para o dispositivo USB `CAFE:0001`:

```ini
jamma_vid=0xCAFE
jamma_pid=0x0001
```

Isto é mantido porque faz com que os botões fixos Coin/Start funcionem corretamente nos jogos.

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

Depois, no MiSTer, o ficheiro específico deste dispositivo `/media/fat/config/kbd_cafe_0001.map` faz:

```text
F13 -> P
F14 -> Left Shift
F15 -> Left Ctrl
F16 -> Left Alt
F17 -> Right Alt
```

Assim, Coin/Start continuam a beneficiar do modo JAMMA, enquanto o botão multifunções volta a comportar-se como teclado normal depois dessa conversão.

## Funções do botão

- **Um clique curto:** aguarda **350 ms** para excluir duplo clique e envia `F13` durante cerca de **200 ms**. No MiSTer, `F13` é convertido em `P` para Pause.
- **Duplo clique:** envia `F12` durante cerca de **80 ms** ao largar o segundo toque curto. Abre/fecha o OSD.
- **Pressão longa (2 segundos ou mais):** envia `F14 + F15 + F16 + F17` durante cerca de **500 ms**. No MiSTer são convertidas em `Left Shift + Left Ctrl + Left Alt + Right Alt`, para abandonar o jogo e regressar ao Zaparoo Frontend.
- A pressão longa dispara uma única vez.
- Depois de uma pressão longa, largar o botão não envia outra ação.
- Primeiro clique curto + segundo toque mantido por 2 segundos ou mais: é enviada apenas a ação longa.
- O input **A1 tem debounce por software de 30 ms**.

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
- Janela de duplo clique: **350 ms**
- Debounce A1: **30 ms**
- Pressão longa: **2000 ms**
- Impulso single (`F13`): **200 ms**
- Impulso double (`F12`): **80 ms**
- Impulso long (`F14-F17`): **500 ms**
