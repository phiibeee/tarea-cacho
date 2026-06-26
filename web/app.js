const API_BASE = "";

const elements = {
    startScreen: document.getElementById('start-screen'),
    gameView: document.getElementById('game-view'),
    startBtn: document.getElementById('start-btn'),
    gameStatus: document.getElementById('game-status'),
    diceArea: document.getElementById('dice-area'),
    actionsList: document.getElementById('actions-list'),
    attemptVal: document.getElementById('attempt-val'),
    rollVal: document.getElementById('roll-val'),
    scoreboardsContainer: document.getElementById('scoreboards-container'),
    gameOverScreen: document.getElementById('game-over-screen'),
    finalScoreVal: document.getElementById('final-score-val'),
    endMessage: document.getElementById('end-message'),
    restartBtn: document.getElementById('restart-btn'),
    headerRestartBtn: document.getElementById('header-restart-btn'),
    endStatus: document.getElementById('end-status'),
    playerOpts: document.querySelectorAll('.player-opt')
};

console.log("Found player options:", elements.playerOpts.length);

let gameState = null;
let selectedDice = new Set();
let playerCount = 1;

// Player Selection
elements.playerOpts.forEach(opt => {
    opt.onclick = () => {
        console.log("Player count selected:", opt.dataset.count);
        elements.playerOpts.forEach(o => o.classList.remove('active'));
        opt.classList.add('active');
        playerCount = parseInt(opt.dataset.count);
    };
});

async function apiRequest(endpoint, method = "GET", body = null) {
    try {
        const options = { method };
        if (body) {
            options.body = JSON.stringify(body);
            options.headers = { "Content-Type": "application/json" };
        }
        const resp = await fetch(`${API_BASE}${endpoint}`, options);
        if (!resp.ok) throw new Error(`API Error: ${resp.statusText}`);
        return await resp.json();
    } catch (err) {
        console.error(err);
        elements.gameStatus.textContent = "Error de conexión con el servidor";
        return null;
    }
}

function updateUI() {
    if (!gameState) return;

    if (gameState.gameStarted) {
        elements.startScreen.classList.add('hidden');
        elements.gameOverScreen.classList.add('hidden');
        elements.gameView.classList.remove('hidden');
        elements.headerRestartBtn.classList.remove('hidden');
        elements.gameStatus.textContent = "Partida en curso";
        elements.gameStatus.classList.add('active');
    } else if (gameState.gameFinished) {
        elements.gameView.classList.add('hidden');
        elements.headerRestartBtn.classList.add('hidden');
        elements.gameOverScreen.classList.remove('hidden');
        elements.gameStatus.textContent = "Partida finalizada";
        elements.gameStatus.classList.remove('active');

        const winnerName = (gameState.winners && gameState.winners.length > 0) ? gameState.winners.join(", ") : "Nadie";
        const winnerScore = gameState.winningScore !== undefined ? gameState.winningScore : 0;
        const winType = gameState.winMethod || "Puntos";

        console.log("Game Finished Data:", { winnerName, winnerScore, winType });

        elements.endStatus.textContent = `¡Partida Terminada! Ganador: ${winnerName}`;
        elements.finalScoreVal.textContent = winnerScore;

        if (winType === "Dormida") {
            elements.endMessage.textContent = `¡${winnerName} ganó por DORMIDA! (Victoria Instantánea)`;
        } else if (winType === "Alalay") {
            elements.endMessage.textContent = `¡${winnerName} ganó por ALALAY! (Marcador de campeonato)`;
        } else {
            if (gameState.winners && gameState.winners.length > 1) {
                elements.endMessage.textContent = `¡Empate entre ${winnerName} con ${winnerScore} puntos!`;
            } else {
                elements.endMessage.textContent = `${winnerName} es el ganador con ${winnerScore} puntos. ¡Felicidades!`;
            }
        }
    }

    elements.attemptVal.textContent = gameState.intento || 1;
    elements.rollVal.textContent = gameState.lanzamiento || 0;

    // Dice
    elements.diceArea.innerHTML = "";
    if (gameState.lanzamiento === 0) {
        const placeholder = document.createElement('div');
        placeholder.className = 'dice-placeholder';
        placeholder.textContent = `¡Jugador ${gameState.currentPlayer} en juego! Lanza los dados para comenzar.`;
        elements.diceArea.appendChild(placeholder);
    } else {
        gameState.dados.forEach((val, idx) => {
            const die = document.createElement('div');
            die.className = 'die';
            die.dataset.val = val;
            if (selectedDice.has(idx)) die.classList.add('selected');

            // Add dots
            for (let i = 0; i < val; i++) {
                const dot = document.createElement('div');
                dot.className = 'dot';
                die.appendChild(dot);
            }

            die.style.animationDelay = `${idx * 0.1}s`;

            die.onclick = () => {
                if (gameState.lanzamiento >= 2) return;
                if (selectedDice.has(idx)) selectedDice.delete(idx);
                else selectedDice.add(idx);
                updateUI();
            };
            elements.diceArea.appendChild(die);
        });
    }

    // Score Boards (Multiplayer)
    elements.scoreboardsContainer.innerHTML = "";
    if (gameState.marcadores) {
        Object.keys(gameState.marcadores).sort((a, b) => {
            if (a === gameState.currentPlayer) return -1;
            if (b === gameState.currentPlayer) return 1;
            return a.localeCompare(b);
        }).forEach(playerName => {
            const marker = gameState.marcadores[playerName];
            const isActive = playerName === gameState.currentPlayer;

            const card = document.createElement('div');
            card.className = `glass-card score-card ${isActive ? 'active-player' : ''}`;

            let cardHtml = `
                <h3>
                    ${playerName}
                    <span class="active-label">TURNO ACTUAL</span>
                </h3>
                <div class="score-container">
                    <div class="score-grid">
                        <!-- Row 1 -->
                        <div class="score-cell">${renderCell(marker, "balas", "balas")}</div>
                        <div class="score-cell">${renderCell(marker, "escalera", "ESCA")}</div>
                        <div class="score-cell">${renderCell(marker, "cuadras", "cuadras")}</div>
                        <!-- Row 2 -->
                        <div class="score-cell">${renderCell(marker, "tontos", "tontos")}</div>
                        <div class="score-cell">${renderCell(marker, "full", "FULL")}</div>
                        <div class="score-cell">${renderCell(marker, "quinas", "quinas")}</div>
                        <!-- Row 3 -->
                        <div class="score-cell">${renderCell(marker, "trenes", "trenes")}</div>
                        <div class="score-cell">${renderCell(marker, "poker", "POKR")}</div>
                        <div class="score-cell">${renderCell(marker, "senas", "senas")}</div>
                    </div>
                    <div class="grande-area">
                        <div class="grande-slot">${renderCell(marker, "grande", "GRANDE 1")}</div>
                        <div class="grande-slot">${renderCell(marker, "grande2", "GRANDE 2")}</div>
                    </div>
                </div>
                <div class="total-score">
                    <span>TOTAL</span>
                    <span>${marker.suma}</span>
                </div>
            `;

            card.innerHTML = cardHtml;
            elements.scoreboardsContainer.appendChild(card);
        });
    }

    // Actions
    elements.actionsList.innerHTML = "";
    const normalActions = gameState.possibleActions.filter(a => a.accion !== "lanzar" || a.seRespeta);
    const lanzarActions = gameState.possibleActions.filter(a => a.accion === "lanzar" && !a.seRespeta);

    // Reroll Button (if in 1st roll)
    if (lanzarActions.length > 0) {
        const rerollBtn = document.createElement('button');
        rerollBtn.className = 'btn-primary';
        rerollBtn.style.gridColumn = "1 / -1";
        const diceCount = selectedDice.size;
        rerollBtn.innerHTML = `<span>Lanzar ${diceCount > 0 ? diceCount : 'Todos'}</span>`;

        rerollBtn.onclick = () => {
            const sortedSelected = Array.from(selectedDice).sort((a, b) => a - b);
            let actionId = -1;

            if (gameState.lanzamiento === 0) {
                // For the first roll, just take the only available lanzar action
                const firstRoll = lanzarActions.find(a => a.accion === "lanzar");
                if (firstRoll) actionId = firstRoll.id;
            } else {
                // For rerolls, find the one that matches selection
                // If none selected, default to "all" (0,1,2,3,4)
                const targetIndices = selectedDice.size === 0 ? [0, 1, 2, 3, 4] : sortedSelected;

                for (let a of lanzarActions) {
                    const actIndices = [...a.indiceDados].sort((a, b) => a - b);
                    if (JSON.stringify(actIndices) === JSON.stringify(targetIndices)) {
                        actionId = a.id;
                        break;
                    }
                }
            }

            if (actionId !== -1) playAction(actionId);
            else alert("Selección no válida para lanzar");
        };
        elements.actionsList.appendChild(rerollBtn);
    }

    // Normal Actions (Anotar, etc)
    normalActions.forEach(action => {
        const btn = document.createElement('button');
        btn.className = 'btn-action';

        let label = action.accion;
        let sub = "";

        if (action.accion === "anotar") {
            label = `Anotar ${action.juego}`;
            sub = `${action.puntos} puntos`;
        } else if (action.accion === "sobre") {
            label = `Sobre ${action.puntos} en ${action.juego}`;
            sub = `Para el 2do intento`;
        } else if (action.accion === "dormida") {
            label = "DORMIDA";
            sub = "¡Victoria inmediata!";
        } else if (action.seRespeta) {
            label = `¡SE RESPETA! (${action.juego})`;
            sub = "Tira 5 de nuevo (Mano)";
        } else {
            label = action.accion;
        }

        btn.innerHTML = `
            <span>${label}</span>
            <span class="sub">${sub}</span>
        `;

        btn.onclick = () => playAction(action.id);
        elements.actionsList.appendChild(btn);
    });
}

async function startGame() {
    selectedDice.clear();
    elements.gameStatus.textContent = "Iniciando...";
    console.log("Starting game with players:", playerCount);
    const resp = await apiRequest("/start", "POST", { players: playerCount });
    if (resp) {
        gameState = await apiRequest("/state");
        updateUI();
    }
}

async function playAction(id) {
    // Visual feedback for rolling
    if (id < 31) {
        document.querySelectorAll('.die').forEach(d => d.classList.add('rolling'));
    }

    const newState = await apiRequest("/play", "POST", { actionId: id });
    if (newState) {
        gameState = newState;
        selectedDice.clear();
        // Small delay to let animation "finish"
        setTimeout(updateUI, 500);
    }
}

async function performReset() {
    const resp = await apiRequest("/reset", "POST", {});
    if (resp) {
        gameState = null;
        elements.gameView.classList.add('hidden');
        elements.gameOverScreen.classList.add('hidden');
        elements.headerRestartBtn.classList.add('hidden');
        elements.startScreen.classList.remove('hidden');
        elements.gameStatus.textContent = "Esperando inicio...";
        elements.gameStatus.classList.remove('active');
    }
}

elements.startBtn.onclick = startGame;
elements.restartBtn.onclick = performReset;
elements.headerRestartBtn.onclick = () => {
    if (confirm("¿Estás seguro de que quieres reiniciar la partida?")) {
        performReset();
    }
};

// Initial check
async function init() {
    console.log("Initializing app...");
    const state = await apiRequest("/state");
    console.log("Initial state:", state);
    if (state && (state.gameStarted || state.gameFinished)) {
        gameState = state;
        updateUI();
    } else {
        console.log("Game not started or finished, showing start screen.");
        elements.headerRestartBtn.classList.add('hidden');
    }
}

init();

function renderCell(marker, key, label) {
    const val = marker[key];
    let displayVal = val;
    if (val === -1) displayVal = '-';
    else if (val === 0) displayVal = 'BORRE';

    return `
        <span class="cell-label">${label}</span>
        <span class="cell-val ${val === -1 ? 'not-set' : 'highlight-score'}">${displayVal}</span>
    `;
}
