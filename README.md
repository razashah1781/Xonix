# Xonix++ | C++ SFML Game Project 🎮

A modern recreation of the classic arcade game **Xonix**, built in **C++** using the **SFML** library. This project was developed as the final assignment for the **Programming Fundamentals Spring 2025** course at **FAST NUCES Islamabad**.

This version of Xonix has been **fully redesigned** with new features such as advanced difficulty levels, a scoring and reward system, multiplayer mode, and more — all implemented as per the university's official project specification.

---

## 📌 Repository

**GitHub**: [https://github.com/razashah1781/Xonix](https://github.com/razashah1781/Xonix)

---

## ✅ Implemented Features (as per University Requirements)

### 🎮 Core Gameplay
- Player movement in 4 directions
- Tile capture logic and rules
- Collision-based death and movement restrictions while constructing

### 🧩 Game Modes
- **Single Player** and **Two Player** modes
- **Start Menu** with options:
  - Start Game
  - Select Difficulty
  - View Scoreboard
- **End Menu** with:
  - Final score display
  - Highlight if it’s a new high score
  - Options: Restart, Main Menu, Exit Game

### 🔥 Difficulty & Game Types
- Easy (2 enemies)
- Medium (4 enemies)
- Hard (6 enemies)
- **Endless Mode**: Adds 2 enemies every 20 seconds

### 🕒 Movement & Time Tracking
- Displays elapsed game time
- Player movement counter (tracked per tile build)
- Enemies speed increases every 20 seconds
- Geometric enemy patterns (zig-zag, circular) after 30 seconds

### 🏆 Scoring & Rewards
- 1 point per tile captured
- ×2 bonus after capturing 10+ tiles in a move (scales to ×4 after multiple streaks)
- Power-ups unlocked at score milestones (50, 70, 100, etc.)
- Power-ups freeze all enemies (and opponent in multiplayer)

### 📄 Scoreboard
- File-based scoreboard system storing **top 5 scores**
- Sorted in descending order, includes time and score
- Automatically updates if new score is in top 5

### 🧑‍🤝‍🧑 Two Player Mode
- Shared board, real-time interaction
- Arrow keys for Player 1, WASD for Player 2
- Individual score tracking & power-ups
- Shared timer, fair collision and tile conflict rules

### 🔊 Bonus Features
- Sound effects for tile capture, bonuses, power-ups
- Background color changes when power-up is activated

---

## 🛠️ How to Build & Run

### Prerequisites
- C++17 or later
- SFML 2.5+
- CMake
- Linux (recommended)

### Installation

```bash
sudo apt update
sudo apt install cmake build-essential libsfml-dev
git clone https://github.com/razashah1781/Xonix.git
cd Xonix
mkdir build && cd build
cmake ..
make
./xonix