/* ════════════════════════════════════════════════════════════════
 *  MOBILE NETWORK TOWER OPTIMIZATION — SIMULATION ENGINE
 *  Port of main.c: Max-Heap, Graph, BFS, Greedy Rebalancing
 * ════════════════════════════════════════════════════════════════ */

(() => {
  "use strict";

  /* ───────── Constants (matching C code) ───────── */
  const GRID_SIZE   = 100;
  const MAX_STEPS   = 20;
  const BFS_HOPS    = 2;
  const GRAPH_RANGE = 45;

  const TOWER_COLORS = [
    "#3ca55c",  /* Alpha   */
    "#2b5fa3",  /* Beta    */
    "#d97a1e",  /* Gamma   */
    "#d14040",  /* Delta   */
    "#6b5cc5",  /* Echo    */
    "#1b8a6b",  /* Foxtrot */
  ];

  const TOWER_ABBRS = ["AL", "BE", "GA", "DE", "EC", "FO"];

  /* ───────── Data Structures ───────── */

  function makeTower(id, x, y, maxLoad, name) {
    return { id, x, y, load: 0, maxLoad, name };
  }

  function makeUser(id, x, y, dx, dy) {
    return { id, x, y, dx, dy, connectedTower: -1 };
  }

  /* ───────── Graph (Adjacency List) ───────── */

  class TowerGraph {
    constructor(n) {
      this.n = n;
      this.adj = Array.from({ length: n }, () => []);
    }

    addEdge(u, v, dist) {
      this.adj[u].push({ neighbor: v, dist });
      this.adj[v].push({ neighbor: u, dist });
    }

    buildFromTowers(towers, range) {
      for (let i = 0; i < towers.length; i++) {
        for (let j = i + 1; j < towers.length; j++) {
          const dx = towers[i].x - towers[j].x;
          const dy = towers[i].y - towers[j].y;
          const d = Math.sqrt(dx * dx + dy * dy);
          if (d <= range) this.addEdge(i, j, d);
        }
      }
    }

    /** BFS — return tower ids reachable within maxHops (excluding start) */
    bfsNearby(startId, maxHops) {
      const dist = new Array(this.n).fill(-1);
      const queue = [startId];
      dist[startId] = 0;
      const result = [];

      while (queue.length) {
        const curr = queue.shift();
        for (const edge of this.adj[curr]) {
          if (dist[edge.neighbor] === -1) {
            dist[edge.neighbor] = dist[curr] + 1;
            if (dist[edge.neighbor] <= maxHops) {
              result.push(edge.neighbor);
              queue.push(edge.neighbor);
            }
          }
        }
      }
      return result;
    }
  }

  /* ───────── Signal Strength ───────── */

  function calcDist(x1, y1, x2, y2) {
    return Math.sqrt((x1 - x2) ** 2 + (y1 - y2) ** 2);
  }

  function signalStrength(tower, ux, uy) {
    let d = calcDist(ux, uy, tower.x, tower.y);
    if (d < 0.01) d = 0.01;
    const base = 100 / (1 + d * d * 0.01);
    const penalty = tower.load / tower.maxLoad;
    return base * (1 - 0.5 * penalty);
  }

  /* ───────── Max-Heap Tower Selection ───────── */

  function selectBestTower(towers, ux, uy) {
    /* Simple heap: build, extract max */
    const heap = [];

    const siftUp = (i) => {
      while (i > 0) {
        const p = (i - 1) >> 1;
        if (heap[i].signal > heap[p].signal) {
          [heap[i], heap[p]] = [heap[p], heap[i]];
          i = p;
        } else break;
      }
    };

    for (const t of towers) {
      if (t.load >= t.maxLoad) continue;
      heap.push({ signal: signalStrength(t, ux, uy), towerId: t.id });
      siftUp(heap.length - 1);
    }

    return heap.length ? heap[0].towerId : -1;
  }

  /* ───────── Greedy Rebalancing ───────── */

  function rebalanceLoad(towers, users, graph, log) {
    for (const t of towers) {
      if (t.load < t.maxLoad) continue;

      const nearby = graph.bfsNearby(t.id, BFS_HOPS);
      let bestN = -1, lowestPct = 101;

      for (const nid of nearby) {
        if (towers[nid].load < towers[nid].maxLoad) {
          const pct = (towers[nid].load / towers[nid].maxLoad) * 100;
          if (pct < lowestPct) { lowestPct = pct; bestN = nid; }
        }
      }

      if (bestN === -1) continue;

      for (const u of users) {
        if (u.connectedTower === t.id) {
          u.connectedTower = bestN;
          t.load--;
          towers[bestN].load++;
          log.push(`[REBALANCE] User ${u.id} moved: ${t.name} → ${towers[bestN].name}`);
          break;
        }
      }
    }
  }

  /* ───────── User Movement ───────── */

  function moveUser(u) {
    u.x += u.dx;
    u.y += u.dy;
    if (u.x < 0 || u.x > GRID_SIZE) { u.dx *= -1; u.x += u.dx * 2; }
    if (u.y < 0 || u.y > GRID_SIZE) { u.dy *= -1; u.y += u.dy * 2; }
  }

  /* ───────── Handoff ───────── */

  function handoff(u, towers, log) {
    const bestId = selectBestTower(towers, u.x, u.y);
    if (bestId === -1) return;

    if (u.connectedTower !== bestId) {
      if (u.connectedTower !== -1) towers[u.connectedTower].load--;
      u.connectedTower = bestId;
      towers[bestId].load++;
      log.push(`[HANDOFF] User ${u.id} → ${towers[bestId].name}`);
    }
  }

  /* ═══════════════════════════════════════════
   *  SIMULATION STATE
   * ═══════════════════════════════════════════ */

  let towers, users, graph, step, autoTimer, logLines;

  function initState() {
    towers = [
      makeTower(0, 20, 20, 3, "Alpha"),
      makeTower(1, 50, 20, 4, "Beta"),
      makeTower(2, 80, 20, 3, "Gamma"),
      makeTower(3, 20, 70, 3, "Delta"),
      makeTower(4, 50, 70, 4, "Echo"),
      makeTower(5, 80, 70, 3, "Foxtrot"),
    ];

    users = [
      makeUser(0, 15, 15,  2.0,  1.5),
      makeUser(1, 45, 18, -1.5,  2.0),
      makeUser(2, 75, 22,  1.0, -1.0),
      makeUser(3, 25, 65,  2.5,  1.0),
      makeUser(4, 55, 72, -2.0, -1.5),
      makeUser(5, 82, 68, -1.0,  2.0),
      makeUser(6, 30, 40,  1.5, -2.0),
    ];

    graph = new TowerGraph(towers.length);
    graph.buildFromTowers(towers, GRAPH_RANGE);

    step = 0;
    logLines = [];

    /* Initial assignment */
    for (const u of users) {
      const best = selectBestTower(towers, u.x, u.y);
      if (best !== -1) {
        u.connectedTower = best;
        towers[best].load++;
        logLines.push(`[INIT] User ${u.id} → ${towers[best].name}`);
      }
    }
  }

  function simulateStep() {
    if (step >= MAX_STEPS) return false;
    step++;

    const stepLog = [];

    /* Move */
    for (const u of users) moveUser(u);

    /* Handoff */
    for (const u of users) handoff(u, towers, stepLog);

    /* Rebalance */
    rebalanceLoad(towers, users, graph, stepLog);

    logLines.push(...stepLog);
    return true;
  }

  /* ═══════════════════════════════════════════
   *  RENDERING — CANVAS
   * ═══════════════════════════════════════════ */

  const canvas = document.getElementById("sim-canvas");
  const ctx    = canvas.getContext("2d");

  /* Scale canvas for high-DPI */
  function resizeCanvas() {
    const rect = canvas.parentElement.getBoundingClientRect();
    const w = rect.width - 32;  /* padding */
    const h = Math.round(w * 0.53);
    canvas.style.width  = w + "px";
    canvas.style.height = h + "px";
    const dpr = window.devicePixelRatio || 1;
    canvas.width  = w * dpr;
    canvas.height = h * dpr;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    return { w, h };
  }

  function gridToCanvas(gx, gy, cw, ch) {
    const pad = 50;
    return {
      cx: pad + (gx / GRID_SIZE) * (cw - 2 * pad),
      cy: pad + (gy / GRID_SIZE) * (ch - 2 * pad),
    };
  }

  function drawCanvas() {
    const { w, h } = resizeCanvas();
    ctx.clearRect(0, 0, w, h);

    /* ── Graph edges (gray lines) ── */
    ctx.strokeStyle = "#c8c4bb";
    ctx.lineWidth = 1.2;
    const drawn = new Set();
    for (let i = 0; i < towers.length; i++) {
      for (const edge of graph.adj[i]) {
        const key = [Math.min(i, edge.neighbor), Math.max(i, edge.neighbor)].join(",");
        if (drawn.has(key)) continue;
        drawn.add(key);
        const a = gridToCanvas(towers[i].x, towers[i].y, w, h);
        const b = gridToCanvas(towers[edge.neighbor].x, towers[edge.neighbor].y, w, h);
        ctx.beginPath();
        ctx.moveTo(a.cx, a.cy);
        ctx.lineTo(b.cx, b.cy);
        ctx.stroke();
      }
    }

    /* ── Signal links (dashed) ── */
    ctx.setLineDash([4, 4]);
    ctx.lineWidth = 1;
    for (const u of users) {
      if (u.connectedTower === -1) continue;
      const t = towers[u.connectedTower];
      const a = gridToCanvas(u.x, u.y, w, h);
      const b = gridToCanvas(t.x, t.y, w, h);
      ctx.strokeStyle = TOWER_COLORS[t.id] + "55";
      ctx.beginPath();
      ctx.moveTo(a.cx, a.cy);
      ctx.lineTo(b.cx, b.cy);
      ctx.stroke();
    }
    ctx.setLineDash([]);

    /* ── Towers ── */
    for (const t of towers) {
      const { cx, cy } = gridToCanvas(t.x, t.y, w, h);
      const r = 22;

      /* outer glow ring */
      ctx.beginPath();
      ctx.arc(cx, cy, r + 4, 0, Math.PI * 2);
      ctx.fillStyle = TOWER_COLORS[t.id] + "22";
      ctx.fill();

      /* main circle */
      ctx.beginPath();
      ctx.arc(cx, cy, r, 0, Math.PI * 2);
      ctx.fillStyle = TOWER_COLORS[t.id];
      ctx.fill();

      /* abbreviation label */
      ctx.fillStyle = "#fff";
      ctx.font = "bold 12px Inter, sans-serif";
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";
      ctx.fillText(TOWER_ABBRS[t.id], cx, cy);

      /* load count below */
      ctx.fillStyle = TOWER_COLORS[t.id];
      ctx.font = "500 11px Inter, sans-serif";
      ctx.fillText(`${t.load}/${t.maxLoad}`, cx, cy + r + 14);
    }

    /* ── Users ── */
    for (const u of users) {
      const { cx, cy } = gridToCanvas(u.x, u.y, w, h);
      const color = u.connectedTower !== -1 ? TOWER_COLORS[u.connectedTower] : "#999";
      const r = 7;

      ctx.beginPath();
      ctx.arc(cx, cy, r, 0, Math.PI * 2);
      ctx.fillStyle = color;
      ctx.fill();

      /* user label */
      ctx.fillStyle = color;
      ctx.font = "500 10px Inter, sans-serif";
      ctx.textAlign = "center";
      ctx.fillText(`U${u.id}`, cx, cy + r + 11);
    }
  }

  /* ═══════════════════════════════════════════
   *  RENDERING — UI
   * ═══════════════════════════════════════════ */

  const cardsEl    = document.getElementById("tower-cards");
  const logEl      = document.getElementById("event-log");
  const counterEl  = document.getElementById("step-counter");
  const btnStep    = document.getElementById("btn-step");
  const btnAuto    = document.getElementById("btn-auto");
  const btnReset   = document.getElementById("btn-reset");

  function buildCards() {
    cardsEl.innerHTML = "";
    for (const t of towers) {
      const card = document.createElement("div");
      card.className = "tower-card";
      card.dataset.tower = t.id;
      card.innerHTML = `
        <div class="tower-card__name">${t.name}</div>
        <div class="tower-card__info">${t.load}/${t.maxLoad} users · ${Math.round(t.load / t.maxLoad * 100)}%</div>
        <div class="tower-card__bar">
          <div class="tower-card__bar-fill" style="width:${(t.load / t.maxLoad) * 100}%"></div>
        </div>`;
      cardsEl.appendChild(card);
    }
  }

  function updateCards() {
    const cards = cardsEl.querySelectorAll(".tower-card");
    cards.forEach((card) => {
      const t = towers[+card.dataset.tower];
      const pct = Math.round((t.load / t.maxLoad) * 100);
      card.querySelector(".tower-card__info").textContent = `${t.load}/${t.maxLoad} users · ${pct}%`;
      card.querySelector(".tower-card__bar-fill").style.width = pct + "%";
    });
  }

  function renderLog() {
    /* Show most recent 60 lines */
    const visible = logLines.slice(-60);
    logEl.innerHTML = visible.map((l) => `<p>${l}</p>`).join("");
    logEl.scrollTop = logEl.scrollHeight;
  }

  function updateStepCounter() {
    counterEl.textContent = `Step ${step} / ${MAX_STEPS}`;
  }

  /* ═══════════════════════════════════════════
   *  CONTROLS
   * ═══════════════════════════════════════════ */

  function doStep() {
    if (simulateStep()) {
      drawCanvas();
      updateCards();
      renderLog();
      updateStepCounter();
    }
    if (step >= MAX_STEPS) stopAuto();
  }

  let isAuto = false;

  function startAuto() {
    if (isAuto) return;
    isAuto = true;
    btnAuto.classList.add("active");
    autoTimer = setInterval(doStep, 600);
  }

  function stopAuto() {
    isAuto = false;
    btnAuto.classList.remove("active");
    clearInterval(autoTimer);
  }

  function resetSim() {
    stopAuto();
    initState();
    buildCards();
    drawCanvas();
    renderLog();
    updateStepCounter();
  }

  btnStep.addEventListener("click", () => { stopAuto(); doStep(); });
  btnAuto.addEventListener("click", () => { isAuto ? stopAuto() : startAuto(); });
  btnReset.addEventListener("click", resetSim);

  /* Handle window resize */
  window.addEventListener("resize", drawCanvas);

  /* ═══════════════════════════════════════════
   *  BOOT
   * ═══════════════════════════════════════════ */
  initState();
  buildCards();
  drawCanvas();
  renderLog();
  updateStepCounter();
})();
