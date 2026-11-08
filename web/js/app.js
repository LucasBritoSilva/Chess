// Este arquivo foi desenvolvido com auxílio de IA e integra a interface web ao núcleo do jogo em C++.
const $ = (id) => document.getElementById(id);
const names = {
  p: "peão",
  r: "torre",
  n: "cavalo",
  b: "bispo",
  q: "dama",
  k: "rei",
};
const symbols = {
  p: "pawn",
  r: "rook",
  n: "knight",
  b: "bishop",
  q: "queen",
  k: "king",
};
const colors = { white: "Brancas", black: "Pretas" };
const cells = [];
let api,
  state,
  selected = null;

function squareName(square) {
  return String.fromCharCode(97 + (square % 8)) + (Math.floor(square / 8) + 1);
}

function pieceSvg(piece) {
  const svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
  svg.setAttribute("viewBox", "0 0 80 80");
  svg.setAttribute("aria-hidden", "true");
  svg.classList.add(piece === piece.toUpperCase() ? "white" : "black");
  const use = document.createElementNS(svg.namespaceURI, "use");
  use.setAttribute("href", `assets/pieces.svg#${symbols[piece.toLowerCase()]}`);
  svg.append(use);
  return svg;
}

function createBoard() {
  for (let rank = 7; rank >= 0; rank--) {
    for (let file = 0; file < 8; file++) {
      const square = rank * 8 + file;
      const button = document.createElement("button");
      button.type = "button";
      button.className = "square";
      button.classList.toggle("dark", (rank + file) % 2 === 0);
      button.dataset.square = squareName(square);
      button.addEventListener("click", () => onSquare(square));
      for (const [label, value] of [
        ["rank", file === 0 ? rank + 1 : ""],
        ["file", rank === 0 ? squareName(square)[0] : ""],
      ]) {
        if (!value) continue;
        const coordinate = document.createElement("span");
        coordinate.className = `coordinate ${label}`;
        coordinate.textContent = value;
        button.append(coordinate);
      }
      cells[square] = button;
      $("board").append(button);
    }
  }
}

function renderBoard() {
  const destinations =
    state?.legalMoves
      .filter((move) => move.from === selected)
      .map((move) => move.to) || [];
  cells.forEach((button, square) => {
    const piece = state?.board[square] || ".";
    button.classList.toggle("occupied", piece !== ".");
    button.classList.toggle("selected", square === selected);
    button.classList.toggle("legal", destinations.includes(square));
    button.classList.toggle(
      "last",
      state?.lastMove?.from === square || state?.lastMove?.to === square,
    );
    button.classList.toggle("check", state?.checkSquare === square);
    button.setAttribute("aria-pressed", String(square === selected));
    const description =
      piece === "."
        ? "vazia"
        : `${names[piece.toLowerCase()]} (${piece === piece.toUpperCase() ? "brancas" : "pretas"})`;
    button.setAttribute(
      "aria-label",
      `${squareName(square)}, ${description}${destinations.includes(square) ? ", destino legal" : ""}`,
    );
    button.querySelector("svg")?.remove();
    if (piece !== ".") button.prepend(pieceSvg(piece));
  });
}

function renderHistory() {
  $("history").replaceChildren();
  let row;
  state.history.forEach((entry, index) => {
    if (index === 0 || state.history[index - 1].number !== entry.number) {
      row = document.createElement("li");
      row.className = "history-row";
      for (const value of [entry.number, "—", "—"]) {
        const cell = document.createElement("span");
        cell.textContent = value;
        row.append(cell);
      }
      $("history").append(row);
    }
    row.children[entry.player === "white" ? 1 : 2].textContent = entry.san;
  });
  $("empty-history").hidden = state.history.length > 0;
  $("history-scroll").scrollTop = $("history-scroll").scrollHeight;
}

function refresh() {
  state = JSON.parse(api.state());
  selected = null;
  renderBoard();
  renderHistory();
  const playing = state.status === "playing";
  const descriptions = {
    checkmate: `${colors[state.winner]} vencem por xeque-mate.`,
    stalemate: "Afogamento: não há jogadas legais, e o rei não está em xeque.",
    insufficient: "Material insuficiente para um xeque-mate.",
    fivefold: "A mesma posição apareceu cinco vezes.",
    seventyfive: "75 lances de cada lado sem captura ou movimento de peão.",
  };
  $("status-label").textContent = playing
    ? `LANCE ${state.fullmove}`
    : "PARTIDA ENCERRADA";
  $("status-title").textContent = playing
    ? `${colors[state.turn]} jogam${state.check ? " — xeque!" : "."}`
    : state.status === "checkmate"
      ? `${colors[state.winner]} vencem!`
      : "Empate.";
  $("status-description").textContent = playing
    ? state.check
      ? "Proteja seu rei para continuar."
      : "Selecione uma peça para ver os movimentos possíveis."
    : descriptions[state.status];
  document
    .querySelector(".status-card")
    .classList.toggle("in-check", playing && state.check);
  $("new-game").disabled = false;
  $("board").setAttribute("aria-busy", "false");
}

function submitMove(move) {
  if (!api.move(move.from, move.to, move.promotion.charCodeAt(0) || 0)) {
    $("feedback").textContent = "Essa jogada não é permitida.";
    return;
  }
  $("feedback").textContent = "";
  refresh();
}

function choosePromotion(moves) {
  $("promotion-options").replaceChildren();
  for (const move of moves) {
    const button = document.createElement("button");
    const piece =
      state.turn === "white" ? move.promotion.toUpperCase() : move.promotion;
    button.append(pieceSvg(piece));
    const label = document.createElement("span");
    label.textContent = names[move.promotion];
    button.append(label);
    button.setAttribute("aria-label", `Promover a ${names[move.promotion]}`);
    button.addEventListener("click", () => {
      $("promotion-dialog").close();
      submitMove(move);
    });
    $("promotion-options").append(button);
  }
  $("promotion-dialog").showModal();
}

function onSquare(square) {
  if (!state || state.status !== "playing") return;
  $("feedback").textContent = "";
  if (square === selected) {
    selected = null;
    renderBoard();
    return;
  }
  const moves = state.legalMoves.filter(
    (move) => move.from === selected && move.to === square,
  );
  if (moves.length > 1) choosePromotion(moves);
  else if (moves.length === 1) submitMove(moves[0]);
  else if (state.legalMoves.some((move) => move.from === square)) {
    selected = square;
    renderBoard();
  } else {
    $("feedback").textContent =
      selected === null
        ? `Selecione uma peça das ${colors[state.turn].toLowerCase()} que possa se mover.`
        : "Destino inválido. Escolha uma das casas destacadas.";
  }
}

$("cancel-promotion").addEventListener("click", () =>
  $("promotion-dialog").close(),
);
$("new-game").addEventListener("click", () => {
  if (
    !window.confirm("Iniciar uma nova partida? O histórico atual será apagado.")
  )
    return;
  api.reset();
  $("feedback").textContent = "";
  refresh();
});

createBoard();
renderBoard();
try {
  const { default: createChess } = await import("../generated/chess.mjs");
  const module = await createChess();
  api = {
    state: module.cwrap("chess_state", "string", []),
    move: module.cwrap("chess_move", "number", ["number", "number", "number"]),
    reset: module.cwrap("chess_reset", null, []),
  };
  refresh();
} catch (error) {
  console.error("Não foi possível iniciar o ChessGame:", error);
  $("status-label").textContent = "NÃO FOI POSSÍVEL INICIAR";
  $("status-title").textContent = "Tabuleiro indisponível";
  $("status-description").textContent =
    "Recarregue a página. Se o problema continuar, consulte as instruções de execução no README.";
  $("board").setAttribute("aria-busy", "false");
}
