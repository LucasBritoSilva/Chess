# ChessGame

Xadrez para **duas pessoas no mesmo computador**, com regras em **C++17**, executadas no navegador por **WebAssembly**. A interface usa HTML5, CSS3 e JavaScript sem frameworks. Não há servidor de jogo, cadastro, inteligência artificial ou dependências externas durante uma partida.

## Começar no Windows

Com Python 3 e Emscripten instalados e ativados:

```powershell
git clone https://github.com/LucasBritoSilva/Chess.git
cd Chess
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/serve.ps1
```

Abra **http://localhost:8080**. Mantenha o terminal do servidor aberto; `Ctrl+C` encerra. Não abra `index.html` diretamente por `file://`: o carregamento do WebAssembly precisa de HTTP. O servidor atende apenas `127.0.0.1`, para uso local. Se a porta estiver ocupada, use `scripts/serve.ps1 -Port 8081`.

O repositório contém o código-fonte. `web/generated/chess.mjs` e `chess.wasm` são produzidos pelo build e ficam fora do Git. **É necessário compilar depois de clonar.** Não há hospedagem pública configurada.

## Funcionalidades

- Tabuleiro 8×8, coordenadas, peças vetoriais próprias e layout responsivo.
- Seleção por clique, destinos legais, destaque do último lance e do rei em xeque.
- Alternância de jogadores, capturas e indicação clara da vez.
- Movimentos de todas as peças, bloqueio de auto-xeque e de captura do rei.
- Xeque, mate, vencedor e afogamento.
- Roque dos dois lados, en passant e promoção com escolha entre dama, torre, bispo e cavalo.
- Histórico em notação algébrica padrão (SAN), incluindo desambiguação, xeque e mate.
- Reinício com confirmação, giro do tabuleiro e navegação por teclado.
- Empate por acordo, material insuficiente básico, repetição e contagem de lances.
- Mensagem de falha de carregamento caso o módulo não possa iniciar.

## Requisitos e configuração

Ambiente validado: Windows, PowerShell 5.1, G++ 16.2.0 do MSYS2 UCRT64, Python 3.14.3, Emscripten **7.0.0** e Node.js **24.19.0** fornecido pelo emsdk.

- **Para compilar a aplicação:** Emscripten. Ele fornece Clang, Node.js e seu próprio Python.
- **Para servir localmente:** Python 3 no `PATH`.
- **Para testes C++ nativos:** G++ compatível com C++17.
- **Para testes de interface:** Node.js 24+ e npm; o script instala jsdom com `npm ci`. Essas dependências não fazem parte do site.
- Navegador atual com WebAssembly, módulos JavaScript, SVG e `<dialog>`.

### MSYS2 / G++

Instale o [MSYS2](https://www.msys2.org/) e execute no terminal **UCRT64**:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
```

Inclua `C:\msys64\ucrt64\bin` no `PATH` da sessão do PowerShell, se necessário:

```powershell
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
g++ --version
python --version
git --version
```

### Emscripten

Instale conforme a [documentação oficial](https://emscripten.org/docs/getting_started/downloads.html). Exemplo reproduzível no PowerShell, fora do projeto:

```powershell
git clone https://github.com/emscripten-core/emsdk.git "$env:USERPROFILE\emsdk"
Set-Location "$env:USERPROFILE\emsdk"
.\emsdk.bat install 7.0.0
.\emsdk.bat activate 7.0.0
.\upstream\emscripten\em++.exe --version
```

Volte à pasta `Chess`. Os scripts procuram `em++` no `PATH`, em `$env:EMSDK`, em `%USERPROFILE%\emsdk`, em `%USERPROFILE%\.codex\tools\emsdk` e em `C:\emsdk`. Também aceitam um caminho explícito:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -EmsdkPath 'D:\Ferramentas\emsdk'
```

A descoberta aceita versões que usam `em++.exe` ou `em++.bat`. Não é necessário mudar permanentemente o `PATH` nem a política de execução do Windows.

## Como jogar

1. As brancas começam. O painel e a marca “Sua vez” mostram quem joga.
2. Clique em uma peça com movimento disponível. Os destinos legais aparecem no tabuleiro.
3. Clique em um destino destacado. Uma captura aparece também na área do jogador.
4. Ao promover um peão, escolha a nova peça no diálogo. Cancelar mantém o peão na origem.
5. Use **Nova partida** para reiniciar ou **Girar tabuleiro** para mudar a orientação.

No teclado: `Tab` entra no tabuleiro; as setas percorrem as casas; `Enter` ou `Espaço` selecionam e jogam; `Escape` cancela a seleção ou fecha um diálogo. As peças são anunciadas com a casa e a cor para leitores de tela. Animações respeitam a preferência por movimento reduzido.

Na notação do histórico, `K` = rei, `Q` = dama, `R` = torre, `B` = bispo e `N` = cavalo; `x` indica captura, `+` xeque e `#` mate.

### Empates

Implementação baseada nos artigos 5 e 9 das [Leis do Xadrez da FIDE](https://handbook.fide.com/chapter/e012023):

- **Afogamento:** encerramento automático quando não há movimento legal e o rei não está em xeque.
- **Material insuficiente básico:** reis isolados; rei e um bispo ou cavalo contra rei; somente reis e bispos, todos esses bispos em casas da mesma cor.
- **Três repetições / 50 lances:** o botão **Reivindicar empate** aparece quando há direito à reivindicação na posição atual ou por um lance anunciado. No segundo caso, escolha o lance no diálogo; o C++ verifica a condição e encerra sem executá-lo.
- **Cinco repetições / 75 lances:** empate automático. Um mate realizado no lance 75 tem prioridade.
- **Acordo:** o diálogo pede a concordância dos dois jogadores, que compartilham o dispositivo.

Os limites de 50 e 75 lances são por jogador, sem movimento de peão nem captura. A repetição considera a vez, a disposição das peças, os direitos de roque e a possibilidade **legal** de en passant.

**Limite deliberado:** não há busca exaustiva de todas as posições mortas possíveis, como bloqueios complexos de peões. Dois cavalos contra rei e bispos em casas de cores diferentes não são declarados automaticamente insuficientes, pois a impossibilidade de forçar mate não equivale à impossibilidade de qualquer mate. Posições mortas fora dos casos cobertos podem ser encerradas por acordo.

## Arquitetura

```text
engine/
  include/Chess.h          Tipos e interfaces independentes da web
  src/Board.cpp           Tabuleiro, ataques, movimentos, FEN e perft
  src/Game.cpp            Partida, resultados, repetição, SAN e estado JSON
  src/wasm_api.cpp        Ponte C para WebAssembly
web/
  index.html              Interface em português e diálogos
  css/style.css           Layout, tabuleiro e responsividade
  js/app.js               Eventos e apresentação do estado recebido
  assets/                 Peças SVG e ícone próprios
  generated/              Build Emscripten, ignorado pelo Git
scripts/                  Build, servidor e testes no PowerShell
tests/                    C++, WebAssembly, DOM e comparação independente
build/                    Executáveis/ambiente de teste, ignorados pelo Git
```

**Fluxo:** clique → chamada C → validação C++ → estado JSON → renderização. O JavaScript filtra a lista de destinos que o C++ forneceu; não calcula ataques, movimentos, turnos, resultados ou condições de empate. A conversão de índices em coordenadas e a tradução de textos são apresentação.

O `Board` gera movimentos candidatos, aplica cada um a uma cópia e descarta os que expõem o próprio rei. `Game` é a entrada validada para jogar. `Board::after()` é uma transição interna que pressupõe um movimento legal. A leitura de FEN existe para testes; não há importador de posições na interface.

### API WebAssembly

| Função | Contrato |
| --- | --- |
| `chess_state()` | JSON com tabuleiro, vez, movimentos legais, xeque, resultado, histórico e possibilidades de empate |
| `chess_move(from, to, promotion)` | Retorna 1 se aceito ou 0 se inválido; um erro não muda a partida |
| `chess_reset()` | Reinicia todo o estado |
| `chess_claim(from, to, promotion)` | Reivindica com lance anunciado; `(-1, -1, 0)` reivindica na posição atual |
| `chess_agree()` | Encerra por acordo, após confirmação na interface |

Índices: `a1 = 0`, `h8 = 63`. Promoção: código ASCII de `q`, `r`, `b` ou `n`; `0` nos demais lances. Maiúsculas representam peças brancas, minúsculas peças pretas e `.` uma casa vazia. O ponteiro de texto retornado vale até a próxima chamada a `chess_state()`; `cwrap` o converte imediatamente em string JavaScript.

## Testes

Suíte completa, incluindo comparação independente:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test-all.ps1 -WithOracle
```

Testes individuais:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test-native.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test-wasm.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test-ui.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test-oracle.ps1
```

Os testes nativos e WebAssembly usam verificações explícitas que lançam erro e retornam código diferente de zero, **inclusive com `NDEBUG` definido**. Não dependem de `assert` do C++.

- **C++:** 130 verificações, incluindo posição inicial, movimentação, capturas, pinos, xeque, mate, afogamento, roques, en passant, quatro promoções, empates, SAN, turnos, rejeição de entradas inválidas e reinício.
- **Perft:** posição inicial até profundidade 4 (**197.281** folhas), Kiwipete até 3 (**97.862**) e final com en passant até 4 (**43.238**).
- **WebAssembly:** mesma suíte C++, API real, serialização JSON, regras especiais e oito partidas determinísticas de até 150 meios-lances.
- **Interface:** eventos e DOM com o WebAssembly real, incluindo seleção, destino inválido, histórico, giro, teclado, mate, cancelamento/reinício, promoção e diálogos de empate.
- **Oráculo opcional:** 2.773 posições e 85.677 transições, comparando movimentos legais, xeque, material insuficiente, FEN resultante e SAN com `chess==1.11.2`, usando semente fixa. A biblioteca fica em um ambiente virtual sob `build/`, exclusivamente para teste.

`test-ui.ps1` usa `npm ci --ignore-scripts` e o lockfile para instalar dependências locais de teste. `test-oracle.ps1` cria um ambiente virtual e instala a versão fixada em `tests/requirements.txt`. As primeiras execuções desses testes exigem internet.

**Limite da validação realizada:** os testes DOM usam jsdom e não renderizam CSS; o comportamento modal é simulado no teste. O acesso ao navegador/desktop da sessão de desenvolvimento estava indisponível. Portanto, não houve inspeção visual real nem validação em dispositivos móveis. Há breakpoints em 850 e 680 px, mas sua aparência ainda exige revisão em um navegador. HTML, CSS, SVG, JavaScript e WebAssembly foram servidos por HTTP local.

## Escopo

A partida existe apenas na memória da aba. Atualizar ou fechar a página perde o progresso. Não há persistência, relógio, análise, multiplayer online ou computador adversário. As regras dizem respeito ao xadrez padrão, não a Chess960 nem a procedimentos de arbitragem presencial.

## Referências técnicas

- [Emscripten: compilador e opções de exportação](https://emscripten.org/docs/tools_reference/emcc.html).
- [jsdom: ambiente DOM para testes](https://github.com/jsdom/jsdom).
- [python-chess: API usada no comparador independente](https://python-chess.readthedocs.io/en/latest/core.html).

Essas bibliotecas de teste não implementam as regras do jogo entregue: o núcleo de produção é o C++ deste repositório.
