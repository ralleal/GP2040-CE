# GP2040-CE v0.7.12 — MiSTer Cabinet Custom

Firmware personalizado baseado no **GP2040-CE v0.7.12** para **Waveshare RP2040-Zero**, criado para utilização numa máquina arcade com **MiSTer FPGA** e **Zaparoo**.

Ficheiro de firmware:

`GP2040-CE_v0.7.12_WaveshareZero_MiSTerCabinet-Custom.uf2`

## Alterações em relação ao firmware original

Esta versão acrescenta um comportamento especial ao botão lógico **A1** quando o GP2040-CE está em **Keyboard Mode**.

Configuração usada no cabinet:

- **GP4 -> A1**
- **GP5 -> sem função**

Com esta configuração, o botão ligado ao **GP4** funciona assim:

- **Um clique curto:** aguarda uma janela de **500 ms** para distinguir um eventual duplo clique; se não houver segundo clique, envia `P` durante cerca de **80 ms**.
- **Duplo clique:** se o segundo toque começar dentro da janela de **500 ms**, envia `F12` ao largar o segundo toque curto. O `P` pendente do primeiro clique é cancelado.
- **Pressão longa (3 segundos ou mais):** envia `Left Shift + Left Ctrl + Left Alt + Right Alt` assim que atinge os **3 segundos**, sem esperar que o botão seja largado.
- A combinação da pressão longa é enviada apenas uma vez, mesmo que o botão continue pressionado durante 6 segundos ou mais.
- Depois de uma pressão longa, largar o botão não envia qualquer outra ação.
- Se houver um primeiro clique curto e o segundo toque for mantido por 3 segundos ou mais, é enviada **apenas** a ação longa; não é enviado `P` nem `F12`.
- O input **A1 tem debounce por software de 30 ms** antes da deteção de press/release, para impedir que o ressalto mecânico do microswitch seja interpretado como cliques adicionais.

Também foi alterado o nome do produto anunciado pelo USB em modo Keyboard para **`GP2040-CE - Fixed Arcade Buttons`**.

## Funções do botão

### Um clique — `P` / Pause

A tecla `P` é usada como **Pause** por muitos cores arcade do MiSTer. Por isso foi escolhida como a ação de um clique para maximizar a compatibilidade sem configuração adicional.

O `P` só é enviado depois de expirarem os **500 ms** da janela de duplo clique. Isto garante que um duplo clique nunca envia primeiro `P` e depois `F12`.

Quando um jogo/core usa outra tecla para Pause, pode ser criado um mapping específico no MiSTer, desde que o core exponha uma função de pausa apropriada.

#### ZX Spectrum

No core **ZX Spectrum** do MiSTer existe uma pausa própria do core em **`F9`**, independente da tecla que o jogo Spectrum use internamente.

Para evitar que o `P` do botão fixo interfira com jogos que usam `P` como controlo (por exemplo esquemas `O/P`), pode ser criado uma vez um **Advanced Input Mapping** específico para este teclado fixo no core Spectrum:

`P -> F9`

Assim, no Spectrum, o botão fixo passa a acionar a pausa do core em vez de enviar a letra `P` ao jogo. Esse mapping é específico do core/dispositivo e não altera o comportamento nos arcades nem num teclado USB normal.

### Duplo clique — `F12`

No MiSTer, `F12` abre o **OSD (On-Screen Display)** do core atualmente em execução.

O `F12` é enviado apenas quando o segundo toque curto é largado. Se o segundo toque for mantido até aos 3 segundos, o duplo clique é cancelado e prevalece a ação de pressão longa.

### Pressão longa — regressar ao Zaparoo

Ao manter o botão pressionado durante **3 segundos**, o firmware envia simultaneamente:

`Left Shift + Left Ctrl + Left Alt + Right Alt`

Neste cabinet, esta combinação foi confirmada com um teclado físico como o atalho para **abandonar o jogo atual e regressar ao Zaparoo Frontend**.

A combinação é enviada durante cerca de **80 ms**, uma única vez por pressão.

## Resumo do comportamento

```text
um clique curto
-> espera 500 ms
-> P
-> Pause nos cores/jogos compatíveis

2 cliques curtos
-> F12
-> OSD/menu do core atual

manter >= 3 segundos
-> Left Shift + Left Ctrl + Left Alt + Right Alt
-> abandonar o jogo / regressar ao Zaparoo Frontend
```

## Importante

O comportamento single/double/long press está implementado diretamente no firmware e **não aparece no Web Configurator**.

O Web Configurator continua apenas a definir qual GPIO corresponde ao botão lógico **A1**. Para este cabinet deve ficar:

`GP4 -> A1`

Os restantes botões continuam a funcionar normalmente e não são afetados por esta alteração.

O valor atribuído a `A1` na página **Keyboard Mapping** do Web Configurator não é usado para esta função custom, porque o firmware interceta diretamente o input lógico `A1`.

O botão físico de **RESET** do cabinet continua separado desta máquina de estados.

## Base

- GP2040-CE: **v0.7.12**
- Board: **Waveshare RP2040-Zero**
- Modo de utilização: **Keyboard**
- Destino: **cabinet MiSTer FPGA / Zaparoo**
- Janela de duplo clique: **500 ms**
- Debounce do A1: **30 ms**
- Pressão longa: **3000 ms**
- Duração dos impulsos de teclado: **80 ms**
