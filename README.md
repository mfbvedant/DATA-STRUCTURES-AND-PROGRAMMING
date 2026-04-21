# 📡 Mobile Network Tower Optimization System

> A C-based simulation of real-world mobile network tower management using core DSA concepts — Graph, BFS, Max-Heap, Greedy Algorithm, and Simulation Loop.

---

## 📌 Table of Contents

- [About the Project](#about-the-project)
- [Problem Statement](#problem-statement)
- [Features](#features)
- [DSA Concepts Used](#dsa-concepts-used)
- [Project Structure](#project-structure)
- [How to Run](#how-to-run)
- [Sample Output](#sample-output)
- [Complexity Analysis](#complexity-analysis)
- [Real-World Applications](#real-world-applications)
- [Author](#author)

---

## 🔍 About the Project

This project simulates a **Mobile Network Tower Optimization System** built entirely in C. It models how towers dynamically manage user connections in real time — assigning users to the best available tower based on signal strength, handling movement (handoff), and redistributing load when towers become overloaded.

The system runs on a **100×100 grid** with:
- **6 Towers** — Alpha, Beta, Gamma, Delta, Echo, Foxtrot
- **7 Moving Users** — each with a position and velocity
- **5 Simulation Steps** — showing dynamic network behavior

---

## ❗ Problem Statement

In a real mobile network:
- Users are constantly moving
- Each tower has a limited capacity
- Signal strength varies with distance and load
- Overloaded towers cause poor connectivity

**How do we assign users to the best tower dynamically while avoiding overload?**

This project answers that question using efficient DSA-backed algorithms.

---

## ✅ Features

- 📶 **Signal-based Tower Selection** — Uses inverse square formula with load penalty
- 🔁 **Dynamic Handoff** — Users switch towers automatically as they move
- ⚖️ **Load Balancing** — Overloaded towers redistribute users via BFS + Greedy
- 🗺️ **Grid Visualization** — ASCII grid shows towers (`T`) and users (`u`) at each step
- 📊 **Tower Dashboard** — Bar chart display of each tower's current load
- 🔢 **BFS Demo** — Shows reachable towers from the most loaded tower at each step

---

## 🧠 DSA Concepts Used

| # | Concept | Where Used | Why |
|---|---------|-----------|-----|
| 1 | **Graph (Adjacency List)** | `graphBuildFromTowers()` | Represents tower network connections |
| 2 | **BFS** | `graphBfsNearby()` | Finds nearby towers for load balancing |
| 3 | **Max-Heap (Priority Queue)** | `selectBestTower()` | Efficiently picks strongest signal tower |
| 4 | **Greedy Algorithm** | `rebalanceLoad()` | Picks least-loaded neighbor during rebalance |
| 5 | **Simulation Loop** | `main()` | Drives real-time user movement and updates |

### Signal Strength Formula
```
signal = (100 / (1 + dist² × 0.01)) × (1 - 0.5 × loadRatio)
```
- Decreases with distance (inverse square)
- Penalized when tower is heavily loaded

---

## 📁 Project Structure

```
mobile-network-optimization/
│
├── main.c          # Complete source code (all sections in one file)
└── README.md       # Project documentation
```

### Code Sections inside `main.c`

```
Section 1 → Constants & Data Structures  (Tower, User structs)
Section 2 → Graph (Adjacency List)       (build, BFS)
Section 3 → Signal Strength & Max-Heap  (selectBestTower)
Section 4 → Load Balancing (Greedy)     (rebalanceLoad)
Section 5 → User Movement Simulation    (moveUser, handoff)
Section 6 → Display / Visualization     (printGrid, printStatus)
Main       → Ties all sections together
```

---

## ▶️ How to Run

### Prerequisites
- GCC compiler
- Linux / macOS / Windows (with MinGW)

### Compile & Run

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/mobile-network-optimization.git
cd mobile-network-optimization

# Compile
gcc main.c -o network_sim -lm

# Run
./network_sim
```

> **Note:** The `-lm` flag is required to link the math library (`sqrt` function).

---

## 🖥️ Sample Output

```
=============================================
  MOBILE NETWORK TOWER OPTIMIZATION SYSTEM
  DSA: Heap | Graph | BFS | Greedy | Sim
=============================================

[GRAPH] Tower connections built (range = 45 units):
  Alpha <-> Beta(d=30.0) Delta(d=50.0)
  ...

[INIT] Assigning users to best towers...
  User 0 -> Alpha
  User 1 -> Beta
  ...

+------+----------+--------+----------------+
|         TOWER STATUS DASHBOARD            |
+------+----------+--------+----------------+
|  ID  |   Name   |  Load  |  Bar           |
+------+----------+--------+----------------+
|    0 |    Alpha |   2/  3 | [######----]  |
|    1 |     Beta |   3/  4 | [########--]  |
...

============== STEP 1 ==============
  [HANDOFF] User 2 -> Tower Gamma
  [REBALANCE] User 1 moved: Beta -> Alpha
  ...
```

---

## 📈 Complexity Analysis

| Operation | Algorithm | Time Complexity |
|-----------|-----------|----------------|
| Tower Selection | Max-Heap | O(log n) |
| BFS Neighbor Search | BFS | O(V + E) |
| Load Rebalancing | Greedy | O(N) per tower |
| Per Simulation Step | Movement + Handoff | O(U) |
| Full Simulation | All steps combined | O(U × steps) |

---

## 🌍 Real-World Applications

- 📱 Mobile networks (Jio, Airtel, Vodafone, 5G)
- 🏙️ Smart city connectivity planning
- 📡 5G tower placement and optimization
- 🌐 Load balancing in distributed networks
- 🚗 Vehicle-to-tower handoff in autonomous driving networks

---

## 👨‍💻 Author

**Your Name**
- GitHub: [@YOUR_USERNAME](https://github.com/YOUR_USERNAME)
- Email: your.email@example.com

---

## 📜 License

This project is open source and available under the [MIT License](LICENSE).

---

> ⭐ If you found this project helpful, please give it a star!
