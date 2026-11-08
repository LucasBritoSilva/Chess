// Presentation only. Legal moves, results, captures and draw claims come from C++.
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
const svgNS = "http://www.w3.org/2000/svg";
let api,
  state,
  selected = null,
  flipped = false,
  focused = 12,
  confirmAction = null;
const cells = new Map();

function squareName(square) {
  return String.fromCharCode(97 + (square % 8)) + (Math.floor(square / 8) + 1);
}
function pieceColor(piece) {
  return piece === piece.toUpperCase() ? "white" : "black";
}
function pieceSvg(piece) {
  const svg = document.createElementNS(svgNS, "svg");
  svg.setAttribute("viewBox", "0 0 80 80");
  svg.setAttribute("aria-hidden", "true");
  svg.classList.add(pieceColor(piece));
  const use = document.createElementNS(svgNS, "use");
  use.setAttribute("href", `assets/pieces.svg#${symbols[piece.toLowerCase()]}`);
  svg.append(use);
  return svg;
}
function displayOrder() {
  return Array.from({ length: 64 }, (_, i) =>
    flipped
      ? Math.floor(i / 8) * 8 + 7 - (i % 8)
      : (7 - Math.floor(i / 8)) * 8 + (i % 8),
  );
}
function createBoard() {
  $("board").replaceChildren();
  cells.clear();
  displayOrder().forEach((square, position) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = "square";
    button.dataset.square = squareName(square);
    button.addEventListener("click", () => onSquare(square));
    button.addEventListener("focus", () => {
      focused = square;
      for (const [s, cell] of cells) cell.tabIndex = s === focused ? 0 : -1;
    });
    button.addEventListener("keydown", (event) => {
      const steps = { ArrowLeft: -1, ArrowRight: 1, ArrowUp: -8, ArrowDown: 8 };
      if (event.key === "Escape") {
        selected = null;
        renderBoard();
      }
      if (!(event.key in steps)) return;
      event.preventDefault();
      const target = position + steps[event.key];
      if (
        target >= 0 &&
        target < 64 &&
        (Math.abs(steps[event.key]) !== 1 ||
          Math.floor(target / 8) === Math.floor(position / 8))
      )
        cells.get(displayOrder()[target]).focus();
    });
    cells.set(square, button);
    $("board").append(button);
  });
}
function renderBoard() {
  const legal = state?.legalMoves.filter((m) => m.from === selected) || [];
  displayOrder().forEach((square, position) => {
    const button = cells.get(square);
    const piece = state?.board[square] || ".";
    button.className = "square";
    button.classList.toggle(
      "dark",
      (Math.floor(square / 8) + (square % 8)) % 2 === 0,
    );
    button.classList.toggle("occupied", piece !== ".");
    button.classList.toggle("selected", square === selected);
    button.classList.toggle(
      "legal",
      legal.some((m) => m.to === square),
    );
    button.classList.toggle(
      "last",
      Boolean(
        state?.lastMove &&
        [state.lastMove.from, state.lastMove.to].includes(square),
      ),
    );
    button.classList.toggle("check", state?.checkSquare === square);
    button.tabIndex = square === focused ? 0 : -1;
    button.setAttribute("aria-pressed", String(square === selected));
    button.setAttribute(
      "aria-label",
      `${squareName(square)}, ${piece === "." ? "vazia" : `${names[piece.toLowerCase()]} ${pieceColor(piece) === "white" ? "branco" : "preto"}`}${legal.some((m) => m.to === square) ? ", destino legal" : ""}`,
    );
    button.replaceChildren();
    if (piece !== ".") button.append(pieceSvg(piece));
    if (position % 8 === 0) {
      const rank = document.createElement("span");
      rank.className = "coordinate rank";
      rank.textContent = squareName(square)[1];
      button.append(rank);
    }
    if (position >= 56) {
      const file = document.createElement("span");
      file.className = "coordinate file";
      file.textContent = squareName(square)[0];
      button.append(file);
    }
  });
}
function refresh() {
  const previousCount = state?.history.length || 0;
  state = JSON.parse(api.state());
  selected = null;
  renderBoard();
  if (state.history.length > previousCount)
    cells
      .get(state.lastMove.to)
      .querySelector("svg")
      ?.classList.add("piece-enter");
  const playing = state.status === "playing";
  for (const color of ["white", "black"]) {
    const active = playing && state.turn === color;
    $(`player-${color}`).classList.toggle("active", active);
    $(`${color}-tag`).hidden = !active;
    const captured = state.history.filter(
      (h) => h.player === color && h.captured !== ".",
    );
    const container = $(`captured-${color}`);
    container.replaceChildren(...captured.map((h) => pieceSvg(h.captured)));
    if (!captured.length) container.textContent = "Nenhuma captura";
    container.setAttribute(
      "aria-label",
      `Capturadas pelas ${colors[color].toLowerCase()}: ${captured.map((h) => names[h.captured.toLowerCase()]).join(", ") || "nenhuma"}`,
    );
  }
  const descriptions = {
    checkmate: `${colors[state.winner]} vencem por xeque-mate.`,
    stalemate: "Afogamento: não há jogadas legais, e o rei não está em xeque.",
    insufficient: "Material insuficiente para um xeque-mate.",
    fivefold: "A mesma posição apareceu cinco vezes.",
    seventyfive: "75 lances de cada lado sem captura ou movimento de peão.",
    claimed: "Empate reivindicado por repetição ou pela regra dos 50 lances.",
    agreement: "Os dois jogadores concordaram com o empate.",
  };
  $("status-label").textContent = playing
    ? `LANCE ${state.fullmove} · ${state.check ? "REI EM XEQUE" : "PARTIDA EM ANDAMENTO"}`
    : "PARTIDA ENCERRADA";
  $("status-title").textContent = playing
    ? `${colors[state.turn]} jogam${state.check ? " — xeque!" : "."}`
    : state.status === "checkmate"
      ? `${colors[state.winner]} vencem!`
      : "Empate.";
  $("status-description").textContent = playing
    ? state.check
      ? "Proteja seu rei para continuar."
      : "O tabuleiro está aberto às suas ideias."
    : descriptions[state.status];
  document
    .querySelector(".status-card")
    .classList.toggle("in-check", playing && state.check);
  document.querySelector(".status-card").classList.toggle("ended", !playing);
  $("history").replaceChildren();
  let row;
  state.history.forEach((entry, index) => {
    if (index === 0 || state.history[index - 1].number !== entry.number) {
      row = document.createElement("li");
      row.className = "history-row";
      for (const value of [String(entry.number).padStart(2, "0"), "—", "—"]) {
        const span = document.createElement("span");
        span.textContent = value;
        row.append(span);
      }
      $("history").append(row);
    }
    const cell = row.children[entry.player === "white" ? 1 : 2];
    cell.textContent = entry.san;
    if (index === state.history.length - 1) cell.className = "latest";
  });
  $("empty-history").hidden = state.history.length > 0;
  $("move-count").textContent =
    `${state.history.length} ${state.history.length === 1 ? "lance" : "lances"}`;
  if (state.history.length !== previousCount)
    $("history-scroll").scrollTop = $("history-scroll").scrollHeight;
  $("new-game").disabled = false;
  $("flip-board").disabled = false;
  $("draw-agreement").disabled = !playing;
  $("claim-draw").hidden = !state.canClaim && !state.claimMoves.length;
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
function onSquare(square) {
  if (!state || state.status !== "playing") return;
  $("feedback").textContent = "";
  if (square === selected) {
    selected = null;
    renderBoard();
    return;
  }
  const candidates = state.legalMoves.filter(
    (m) => m.from === selected && m.to === square,
  );
  if (candidates.length) {
    if (candidates.length === 1) submitMove(candidates[0]);
    else {
      $("promotion-options").replaceChildren();
      for (const move of candidates) {
        const button = document.createElement("button");
        const piece =
          state.turn === "white"
            ? move.promotion.toUpperCase()
            : move.promotion;
        button.append(pieceSvg(piece));
        const label = document.createElement("span");
        label.textContent = names[move.promotion];
        button.append(label);
        button.setAttribute(
          "aria-label",
          `Promover a ${names[move.promotion]}`,
        );
        button.addEventListener("click", () => {
          $("promotion-dialog").close();
          submitMove(move);
        });
        $("promotion-options").append(button);
      }
      $("promotion-dialog").showModal();
    }
  } else if (state.legalMoves.some((m) => m.from === square)) {
    selected = square;
    renderBoard();
  } else
    $("feedback").textContent =
      selected === null
        ? `Selecione uma peça das ${colors[state.turn].toLowerCase()} que possa se mover.`
        : "Destino inválido. Escolha uma das casas destacadas.";
}
function confirm(title, description, action, label) {
  $("confirm-title").textContent = title;
  $("confirm-description").textContent = description;
  $("confirm-ok").textContent = label;
  confirmAction = action;
  $("confirm-dialog").showModal();
  $("confirm-cancel").focus();
}
$("confirm-cancel").addEventListener("click", () =>
  $("confirm-dialog").close(),
);
$("confirm-dialog").addEventListener("close", () => {
  confirmAction = null;
});
$("confirm-ok").addEventListener("click", () => {
  const action = confirmAction;
  $("confirm-dialog").close();
  action?.();
});
$("cancel-promotion").addEventListener("click", () =>
  $("promotion-dialog").close(),
);
$("new-game").addEventListener("click", () =>
  confirm(
    "Uma nova partida?",
    "O tabuleiro e o histórico serão reiniciados. As brancas jogam primeiro.",
    () => {
      api.reset();
      $("feedback").textContent = "";
      refresh();
    },
    "Começar de novo",
  ),
);
$("flip-board").addEventListener("click", () => {
  flipped = !flipped;
  createBoard();
  renderBoard();
  $("feedback").textContent =
    `Tabuleiro virado: ${flipped ? "pretas" : "brancas"} na parte inferior.`;
});
$("draw-agreement").addEventListener("click", () =>
  confirm(
    "Empate por acordo",
    "Brancas e pretas concordam em encerrar a partida? Confirme apenas quando os dois jogadores aceitarem.",
    () => {
      api.agree();
      $("feedback").textContent = "";
      refresh();
    },
    "Ambos concordam",
  ),
);
$("claim-draw").addEventListener("click", () => {
  $("claim-move").replaceChildren();
  if (state.canClaim) {
    const option = new Option("A posição atual já permite empate", "current");
    $("claim-move").append(option);
  }
  state.claimMoves.forEach((m, index) =>
    $("claim-move").append(
      new Option(
        `${squareName(m.from)} → ${squareName(m.to)}${m.promotion ? `, ${names[m.promotion]}` : ""}`,
        String(index),
      ),
    ),
  );
  $("claim-dialog").showModal();
});
$("claim-cancel").addEventListener("click", () => $("claim-dialog").close());
$("claim-ok").addEventListener("click", () => {
  const selection = $("claim-move").value;
  const move =
    selection === "current" ? null : state.claimMoves[Number(selection)];
  const accepted = move
    ? api.claim(move.from, move.to, move.promotion.charCodeAt(0) || 0)
    : api.claim(-1, -1, 0);
  $("claim-dialog").close();
  $("feedback").textContent = accepted
    ? ""
    : "Ainda não é possível reivindicar empate.";
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
    claim: module.cwrap("chess_claim", "number", [
      "number",
      "number",
      "number",
    ]),
    agree: module.cwrap("chess_agree", "number", []),
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
