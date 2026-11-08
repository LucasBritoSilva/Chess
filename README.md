# ChessGame

Um jogo de xadrez para duas pessoas no mesmo computador. O projeto usa C++ para as regras e WebAssembly para executá-las no navegador. A interface é feita com HTML, CSS e JavaScript, sem frameworks.

## Funcionalidades

- Movimentos legais de todas as peças, capturas e alternância de jogadores.
- Proteção contra movimentos que deixam o próprio rei em xeque.
- Xeque, xeque-mate e indicação do vencedor.
- Roque pequeno e grande, en passant e promoção com escolha da peça.
- Empate por afogamento ou material insuficiente básico.
- Destaque da peça selecionada, dos destinos possíveis e do último lance.
- Histórico de jogadas e botão para começar uma nova partida.
- Layout que se adapta a telas menores.

## Tecnologias

- **C++17:** tabuleiro, movimentos, regras, histórico e resultado.
- **WebAssembly e Emscripten:** compilação e execução do C++ no navegador.
- **HTML5 e CSS3:** estrutura e aparência da página.
- **JavaScript:** cliques, seleção, chamadas ao C++ e atualização da interface.

O JavaScript não calcula movimentos nem decide resultados. Ele usa os dados enviados pelo C++. O comentário no início de `app.js` registra o auxílio de IA no desenvolvimento desse arquivo.

## Estrutura do projeto

```text
engine/
  include/       Tipos e interfaces do núcleo
  src/           Tabuleiro, partida e integração WebAssembly
web/
  index.html
  css/
  js/
  assets/        Peças e ícone em SVG
  generated/     Arquivos gerados pela compilação, fora do Git
scripts/         Comandos de build, execução e testes
tests/          Testes do núcleo e da interface
```

FEN foi mantido para montar posições de teste. O histórico usa SAN, uma notação curta como `e4`, `Nf3`, `O-O` ou `Qh4#`. Os dois recursos já são pequenos e ajudam a verificar o jogo, sem acrescentar controles à interface.

## Como executar

No Windows, tenha Git, Python 3 e Emscripten instalados. Para os testes nativos, também é necessário G++ com suporte a C++17; o projeto foi testado com o G++ do MSYS2 UCRT64.

Para instalar a versão do Emscripten usada no projeto, execute no PowerShell:

```powershell
git clone https://github.com/emscripten-core/emsdk.git "$env:USERPROFILE\emsdk"
Set-Location "$env:USERPROFILE\emsdk"
.\emsdk.bat install 7.0.0
.\emsdk.bat activate 7.0.0
.\upstream\emscripten\em++.exe --version
```

O emsdk inclui Node.js e Python para a compilação. O comando `python` também precisa estar disponível no terminal para iniciar o servidor local. Mais opções de instalação estão na [documentação do Emscripten](https://emscripten.org/docs/getting_started/downloads.html).

Na pasta onde você guarda seus projetos:

```powershell
git clone https://github.com/LucasBritoSilva/Chess.git
cd Chess
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/serve.ps1
```

Abra **http://localhost:8080**. Deixe o terminal aberto enquanto joga e use `Ctrl+C` para encerrar o servidor. Se a porta estiver ocupada, passe `-Port 8081` ao script de execução.

O build procura o Emscripten no `PATH`, em `$env:EMSDK`, em `%USERPROFILE%\emsdk`, em `%USERPROFILE%\.codex\tools\emsdk` ou em `C:\emsdk`. Se estiver em outro lugar, informe o caminho:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -EmsdkPath 'D:\Ferramentas\emsdk'
```

Compile novamente depois de alterar o C++. Os arquivos `chess.mjs` e `chess.wasm` não são versionados. Abrir `index.html` diretamente por `file://` não substitui o servidor HTTP.

## Como jogar

As brancas começam. Clique em uma peça para ver os destinos legais e depois clique no destino escolhido. Para cancelar a seleção, clique outra vez na mesma peça. O painel mostra de quem é a vez e avisa quando há xeque ou quando a partida termina.

Na promoção, escolha dama, torre, bispo ou cavalo. **Nova partida** pede confirmação antes de limpar o tabuleiro e o histórico. Também é possível usar Tab e Enter ou Espaço nos botões, com a navegação padrão do navegador.

No histórico, `K` é rei, `Q` dama, `R` torre, `B` bispo e `N` cavalo. `x` indica captura, `+` xeque e `#` mate.

## Testes

Para executar os testes C++, WebAssembly e interface, na pasta do projeto:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test-all.ps1
```

O G++ precisa estar no `PATH`. Em uma instalação padrão do MSYS2, você pode configurá-lo para a sessão atual com:

```powershell
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
```

Os testes da interface usam Node.js 24+ e jsdom. O script encontra o Node do emsdk e instala as dependências de teste com `npm ci`. Elas não são usadas pelo jogo.

Para incluir a comparação independente com python-chess:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test-all.ps1 -WithOracle
```

Essa opção cria um ambiente Python dentro de `build/`. A primeira instalação das dependências de teste precisa de internet.

A revisão passou em **124 verificações C++**, executadas nativamente e em WebAssembly, incluindo contagem de posições (perft), movimentos especiais, xeque, mate, empates e reinício. As verificações continuam ativas com `NDEBUG`. Também passaram os testes da comunicação com JavaScript, oito partidas determinísticas e os testes DOM de interação. A comparação independente conferiu **2.773 posições e 85.677 transições e notações**, sem divergências.

## Limitações

- Os empates implementados são afogamento e material insuficiente básico. Não há regra dos 50 ou 75 lances, repetição de posição, reivindicação ou botão de acordo.
- Material insuficiente cobre reis isolados; rei e um bispo ou cavalo contra rei; e posições só com reis e bispos, todos os bispos em casas da mesma cor. Posições mortas mais complexas não são detectadas automaticamente.
- A partida fica na memória da aba. Recarregar ou fechar a página perde o progresso. Não há opção de desfazer.
- O jogo é local: não tem adversário controlado pelo computador nem partidas online.
- A revisão da interface foi feita com testes DOM e WebAssembly real. O acesso ao navegador estava indisponível nesta sessão, então a aparência em desktop e celular ainda não foi inspecionada visualmente. Os testes DOM simulam os diálogos e não verificam a renderização do CSS.
