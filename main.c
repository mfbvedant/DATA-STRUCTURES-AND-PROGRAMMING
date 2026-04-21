/*
 * ============================================================
 *   MOBILE NETWORK TOWER OPTIMIZATION SYSTEM
 *   Built with DSA concepts in C
 * ============================================================
 *
 *  DSA CONCEPTS USED:
 *  1. Priority Queue (Max-Heap)  -> Best tower selection
 *  2. Graph (Adjacency List)     -> Tower network map
 *  3. BFS (Breadth-First Search) -> Nearby tower discovery
 *  4. Greedy Algorithm           -> Load balancing
 *  5. Simulation Loop            -> User movement
 * ============================================================
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ─────────────────────────────────────────────
 *  SECTION 1: CONSTANTS & DATA STRUCTURES
 * ───────────────────────────────────────────── */

#define MAX_TOWERS 10
#define MAX_USERS 20
#define MAX_EDGES 50 /* max edges per tower */
#define MAX_NAME_LEN 16
#define GRID_SIZE 100.0
#define BFS_QUEUE_SIZE 100

typedef struct {
  int id;
  double x, y; /* Position on grid       */
  int load;    /* Current connected users */
  int maxLoad; /* Maximum capacity        */
  char name[MAX_NAME_LEN];
} Tower;

typedef struct {
  int id;
  double x, y;        /* Current position           */
  double dx, dy;      /* Movement direction (velocity) */
  int connectedTower; /* Tower ID (-1 if none)      */
} User;

/* ── Helper functions for Tower ── */
int towerIsOverloaded(const Tower *t) { return t->load >= t->maxLoad; }
double towerLoadPercent(const Tower *t) {
  return (double)t->load / t->maxLoad * 100.0;
}

/* Initialise a Tower struct */
void initTower(Tower *t, int id, double x, double y, int maxLoad,
               const char *name) {
  t->id = id;
  t->x = x;
  t->y = y;
  t->load = 0;
  t->maxLoad = maxLoad;
  strncpy(t->name, name, MAX_NAME_LEN - 1);
  t->name[MAX_NAME_LEN - 1] = '\0';
}

/* Initialise a User struct */
void initUser(User *u, int id, double x, double y, double dx, double dy) {
  u->id = id;
  u->x = x;
  u->y = y;
  u->dx = dx;
  u->dy = dy;
  u->connectedTower = -1;
}

/* ─────────────────────────────────────────────
 *  SECTION 2: GRAPH (ADJACENCY LIST)
 *  Towers are nodes; edges = towers within range
 * ───────────────────────────────────────────── */

typedef struct {
  int neighbor;
  double dist;
} Edge;

typedef struct {
  int numTowers;
  Edge adj[MAX_TOWERS][MAX_EDGES]; /* adjacency list   */
  int adjCount[MAX_TOWERS];        /* edges per tower  */
} TowerGraph;

void graphInit(TowerGraph *g, int numTowers) {
  g->numTowers = numTowers;
  for (int i = 0; i < numTowers; i++)
    g->adjCount[i] = 0;
}

/* Add undirected edge between two towers */
void graphAddEdge(TowerGraph *g, int u, int v, double dist) {
  int cu = g->adjCount[u];
  int cv = g->adjCount[v];
  if (cu < MAX_EDGES) {
    g->adj[u][cu].neighbor = v;
    g->adj[u][cu].dist = dist;
    g->adjCount[u]++;
  }
  if (cv < MAX_EDGES) {
    g->adj[v][cv].neighbor = u;
    g->adj[v][cv].dist = dist;
    g->adjCount[v]++;
  }
}

/* Build graph: connect towers within a given range */
void graphBuildFromTowers(TowerGraph *g, const Tower towers[], int numTowers,
                          double range) {
  for (int i = 0; i < numTowers; i++) {
    for (int j = i + 1; j < numTowers; j++) {
      double dx = towers[i].x - towers[j].x;
      double dy = towers[i].y - towers[j].y;
      double dist = sqrt(dx * dx + dy * dy);
      if (dist <= range) {
        graphAddEdge(g, i, j, dist);
      }
    }
  }
}

/* ── BFS: Find all towers reachable within 'maxHops' steps ──
 * Returns count of reachable towers; fills 'result' array     */
int graphBfsNearby(const TowerGraph *g, int startId, int maxHops, int result[],
                   int resultMax) {
  int resultCount = 0;
  int dist[MAX_TOWERS];
  int queue[BFS_QUEUE_SIZE];
  int front = 0, back = 0;

  for (int i = 0; i < g->numTowers; i++)
    dist[i] = -1;

  dist[startId] = 0;
  queue[back++] = startId;

  while (front < back) {
    int curr = queue[front++];

    for (int e = 0; e < g->adjCount[curr]; e++) {
      int neighbor = g->adj[curr][e].neighbor;
      if (dist[neighbor] == -1) {
        dist[neighbor] = dist[curr] + 1;
        if (dist[neighbor] <= maxHops) {
          if (resultCount < resultMax)
            result[resultCount++] = neighbor;
          if (back < BFS_QUEUE_SIZE)
            queue[back++] = neighbor;
        }
      }
    }
  }
  return resultCount;
}

/* ─────────────────────────────────────────────
 *  SECTION 3: SIGNAL STRENGTH & HEAP SELECTION
 *  Signal drops with distance (inverse square)
 *  We use a max-heap to find best tower fast
 * ───────────────────────────────────────────── */

double calcDistance(double x1, double y1, double x2, double y2) {
  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

/* Signal strength: higher = better.  Penalizes overloaded towers. */
double signalStrength(const Tower *t, double userX, double userY) {
  double dist = calcDistance(userX, userY, t->x, t->y);
  if (dist < 0.01)
    dist = 0.01; /* avoid division by zero */

  double baseSignal = 100.0 / (1.0 + dist * dist * 0.01);
  double loadPenalty = (double)t->load / t->maxLoad; /* 0 to 1 */
  return baseSignal * (1.0 - 0.5 * loadPenalty);
}

/* ── Max-Heap (array-based priority queue) ──
 * Entries are (signal, tower_id).  We pick the highest signal. */

typedef struct {
  double signal;
  int towerId;
} HeapEntry;

/* Swap two heap entries */
static void heapSwap(HeapEntry *a, HeapEntry *b) {
  HeapEntry tmp = *a;
  *a = *b;
  *b = tmp;
}

/* Sift up after insert */
static void heapSiftUp(HeapEntry heap[], int idx) {
  while (idx > 0) {
    int parent = (idx - 1) / 2;
    if (heap[idx].signal > heap[parent].signal) {
      heapSwap(&heap[idx], &heap[parent]);
      idx = parent;
    } else
      break;
  }
}

/* Sift down after extracting root */
static void heapSiftDown(HeapEntry heap[], int size, int idx) {
  while (1) {
    int largest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < size && heap[left].signal > heap[largest].signal)
      largest = left;
    if (right < size && heap[right].signal > heap[largest].signal)
      largest = right;

    if (largest != idx) {
      heapSwap(&heap[idx], &heap[largest]);
      idx = largest;
    } else
      break;
  }
}

/* Select best tower using max-heap (priority queue) */
int selectBestTower(const Tower towers[], int numTowers, double userX,
                    double userY) {
  HeapEntry heap[MAX_TOWERS];
  int heapSize = 0;

  /* Insert all non-overloaded towers into heap */
  for (int i = 0; i < numTowers; i++) {
    if (!towerIsOverloaded(&towers[i])) {
      heap[heapSize].signal = signalStrength(&towers[i], userX, userY);
      heap[heapSize].towerId = towers[i].id;
      heapSiftUp(heap, heapSize);
      heapSize++;
    }
  }

  if (heapSize == 0)
    return -1;            /* all towers overloaded */
  return heap[0].towerId; /* max-heap root = best  */
}

/* ─────────────────────────────────────────────
 *  SECTION 4: LOAD BALANCING (GREEDY)
 *  When a tower is overloaded, redistribute
 *  users to nearest available neighbors (BFS)
 * ───────────────────────────────────────────── */

void rebalanceLoad(Tower towers[], int numTowers, User users[], int numUsers,
                   const TowerGraph *graph) {
  for (int ti = 0; ti < numTowers; ti++) {
    if (!towerIsOverloaded(&towers[ti]))
      continue;

    /* Find nearby towers via BFS (within 2 hops) */
    int nearby[MAX_TOWERS];
    int nearbyCount =
        graphBfsNearby(graph, towers[ti].id, 2, nearby, MAX_TOWERS);

    /* Greedy: pick neighbor with lowest load % */
    int bestNeighbor = -1;
    double lowestLoad = 101.0;
    for (int n = 0; n < nearbyCount; n++) {
      int nid = nearby[n];
      if (!towerIsOverloaded(&towers[nid])) {
        double lp = towerLoadPercent(&towers[nid]);
        if (lp < lowestLoad) {
          lowestLoad = lp;
          bestNeighbor = nid;
        }
      }
    }

    if (bestNeighbor == -1)
      continue;

    /* Move ONE user from overloaded tower to best neighbor */
    for (int ui = 0; ui < numUsers; ui++) {
      if (users[ui].connectedTower == towers[ti].id) {
        users[ui].connectedTower = bestNeighbor;
        towers[ti].load--;
        towers[bestNeighbor].load++;
        printf("  [REBALANCE] User %d moved: Tower %s -> %s\n", users[ui].id,
               towers[ti].name, towers[bestNeighbor].name);
        break; /* move one at a time per iteration */
      }
    }
  }
}

/* ─────────────────────────────────────────────
 *  SECTION 5: USER MOVEMENT SIMULATION
 *  Each step, users move by their velocity
 *  and may switch towers (handoff)
 * ───────────────────────────────────────────── */

void moveUser(User *u) {
  u->x += u->dx;
  u->y += u->dy;

  /* Bounce off walls */
  if (u->x < 0 || u->x > GRID_SIZE) {
    u->dx *= -1;
    u->x += u->dx * 2;
  }
  if (u->y < 0 || u->y > GRID_SIZE) {
    u->dy *= -1;
    u->y += u->dy * 2;
  }
}

/* Check if user should switch towers (handoff) */
void handoff(User *u, Tower towers[], int numTowers) {
  int bestId = selectBestTower(towers, numTowers, u->x, u->y);
  if (bestId == -1)
    return;

  if (u->connectedTower != bestId) {
    /* Disconnect from old tower */
    if (u->connectedTower != -1) {
      towers[u->connectedTower].load--;
    }
    /* Connect to new tower */
    u->connectedTower = bestId;
    towers[bestId].load++;
    printf("  [HANDOFF] User %d -> Tower %s\n", u->id, towers[bestId].name);
  }
}

/* ─────────────────────────────────────────────
 *  SECTION 6: DISPLAY / VISUALIZATION
 * ───────────────────────────────────────────── */

void printGrid(const Tower towers[], int numTowers, const User users[],
               int numUsers) {
  const int ROWS = 20, COLS = 40;
  char grid[20][41]; /* 40 chars + null terminator */

  for (int r = 0; r < ROWS; r++) {
    memset(grid[r], '.', COLS);
    grid[r][COLS] = '\0';
  }

  /* Place towers on grid */
  for (int i = 0; i < numTowers; i++) {
    int r = (int)(towers[i].y / GRID_SIZE * (ROWS - 1));
    int c = (int)(towers[i].x / GRID_SIZE * (COLS - 1));
    if (r < 0)
      r = 0;
    if (r >= ROWS)
      r = ROWS - 1;
    if (c < 0)
      c = 0;
    if (c >= COLS)
      c = COLS - 1;
    grid[r][c] = 'T';
  }

  /* Place users on grid */
  for (int i = 0; i < numUsers; i++) {
    int r = (int)(users[i].y / GRID_SIZE * (ROWS - 1));
    int c = (int)(users[i].x / GRID_SIZE * (COLS - 1));
    if (r < 0)
      r = 0;
    if (r >= ROWS)
      r = ROWS - 1;
    if (c < 0)
      c = 0;
    if (c >= COLS)
      c = COLS - 1;
    if (grid[r][c] == '.')
      grid[r][c] = 'u';
  }

  printf("\n+");
  for (int i = 0; i < COLS; i++)
    printf("-");
  printf("+\n");
  for (int r = 0; r < ROWS; r++)
    printf("|%s|\n", grid[r]);
  printf("+");
  for (int i = 0; i < COLS; i++)
    printf("-");
  printf("+\n");
  printf("  T = Tower,  u = User,  . = Empty\n\n");
}

void printStatus(const Tower towers[], int numTowers) {
  printf("\n+-------------------------------------------+\n");
  printf("|         TOWER STATUS DASHBOARD            |\n");
  printf("+------+----------+--------+----------------+\n");
  printf("|  ID  |   Name   |  Load  |  Bar           |\n");
  printf("+------+----------+--------+----------------+\n");

  for (int i = 0; i < numTowers; i++) {
    int bars = (int)(towerLoadPercent(&towers[i]) / 10);
    if (bars > 10)
      bars = 10;
    if (bars < 0)
      bars = 0;

    char bar[12];
    for (int b = 0; b < 10; b++)
      bar[b] = (b < bars) ? '#' : '-';
    bar[10] = '\0';

    const char *status = towerIsOverloaded(&towers[i]) ? " FULL" : "     ";
    printf("|  %3d | %8s | %3d/%3d | [%s]%s |\n", towers[i].id, towers[i].name,
           towers[i].load, towers[i].maxLoad, bar, status);
  }

  printf("+------+----------+--------+----------------+\n");
}

void printBFSResult(int startId, const int nearby[], int nearbyCount,
                    const Tower towers[]) {
  printf("\n[BFS] Towers reachable from Tower %s:\n  -> ",
         towers[startId].name);
  if (nearbyCount == 0) {
    printf("(none)\n");
    return;
  }
  for (int i = 0; i < nearbyCount; i++)
    printf("%s ", towers[nearby[i]].name);
  printf("\n");
}

/* ─────────────────────────────────────────────
 *  MAIN: TIE IT ALL TOGETHER
 * ───────────────────────────────────────────── */

int main(void) {
  printf("=============================================\n");
  printf("  MOBILE NETWORK TOWER OPTIMIZATION SYSTEM  \n");
  printf("  DSA: Heap | Graph | BFS | Greedy | Sim    \n");
  printf("=============================================\n\n");

  /* ── Step 1: Initialize Towers ── */
  Tower towers[6];
  int numTowers = 6;
  initTower(&towers[0], 0, 20, 20, 3, "Alpha");
  initTower(&towers[1], 1, 50, 20, 4, "Beta");
  initTower(&towers[2], 2, 80, 20, 3, "Gamma");
  initTower(&towers[3], 3, 20, 70, 3, "Delta");
  initTower(&towers[4], 4, 50, 70, 4, "Echo");
  initTower(&towers[5], 5, 80, 70, 3, "Foxtrot");

  /* ── Step 2: Build Graph (connect towers within range 45) ── */
  TowerGraph graph;
  graphInit(&graph, numTowers);
  graphBuildFromTowers(&graph, towers, numTowers, 45.0);

  printf("[GRAPH] Tower connections built (range = 45 units):\n");
  for (int i = 0; i < numTowers; i++) {
    printf("  %s <-> ", towers[i].name);
    for (int e = 0; e < graph.adjCount[i]; e++)
      printf("%s(d=%.1f) ", towers[graph.adj[i][e].neighbor].name,
             graph.adj[i][e].dist);
    printf("\n");
  }

  /* ── BFS demo: find towers near Alpha ── */
  int nearby[MAX_TOWERS];
  int nearbyCount = graphBfsNearby(&graph, 0, 1, nearby, MAX_TOWERS);
  printBFSResult(0, nearby, nearbyCount, towers);

  /* ── Step 3: Initialize Users ── */
  User users[7];
  int numUsers = 7;
  initUser(&users[0], 0, 15, 15, 2.0, 1.5);
  initUser(&users[1], 1, 45, 18, -1.5, 2.0);
  initUser(&users[2], 2, 75, 22, 1.0, -1.0);
  initUser(&users[3], 3, 25, 65, 2.5, 1.0);
  initUser(&users[4], 4, 55, 72, -2.0, -1.5);
  initUser(&users[5], 5, 82, 68, -1.0, 2.0);
  initUser(&users[6], 6, 30, 40, 1.5, -2.0);

  /* Initial tower assignment */
  printf("\n[INIT] Assigning users to best towers...\n");
  for (int i = 0; i < numUsers; i++) {
    int best = selectBestTower(towers, numTowers, users[i].x, users[i].y);
    if (best != -1) {
      users[i].connectedTower = best;
      towers[best].load++;
      printf("  User %d -> %s\n", users[i].id, towers[best].name);
    }
  }

  printStatus(towers, numTowers);
  printGrid(towers, numTowers, users, numUsers);

  /* ── Step 4: Simulation Loop ── */
  int STEPS = 5;
  printf("\n[SIM] Running %d simulation steps...\n", STEPS);

  for (int step = 1; step <= STEPS; step++) {
    printf("\n============== STEP %d ==============\n", step);

    /* Move all users */
    for (int i = 0; i < numUsers; i++)
      moveUser(&users[i]);

    /* Handoff check */
    for (int i = 0; i < numUsers; i++)
      handoff(&users[i], towers, numTowers);

    /* Rebalance overloaded towers using BFS + greedy */
    rebalanceLoad(towers, numTowers, users, numUsers, &graph);

    /* Status display every 2 steps */
    if (step % 2 == 0 || step == STEPS) {
      printStatus(towers, numTowers);
      printGrid(towers, numTowers, users, numUsers);

      /* BFS from most loaded tower */
      int maxLoad = -1, maxId = 0;
      for (int i = 0; i < numTowers; i++) {
        if (towers[i].load > maxLoad) {
          maxLoad = towers[i].load;
          maxId = towers[i].id;
        }
      }
      int bfsResult[MAX_TOWERS];
      int bfsCount = graphBfsNearby(&graph, maxId, 2, bfsResult, MAX_TOWERS);
      printBFSResult(maxId, bfsResult, bfsCount, towers);
    }
  }

  printf("\n[DONE] Simulation complete!\n");
  printf("============================================\n");
  printf("  DSA Summary:\n");
  printf("  * Max-Heap (priority queue) -> O(log n) tower selection\n");
  printf("  * Graph (adjacency list)    -> O(V + E) BFS traversal\n");
  printf("  * BFS                       -> O(V + E) neighbor search\n");
  printf("  * Greedy rebalancing        -> O(N) per overloaded tower\n");
  printf("  * Movement simulation       -> O(U) per step\n");
  printf("============================================\n");

  return 0;
}