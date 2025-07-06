#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <time.h>
#include <string>
#include <cmath>
#include <cstdlib>
#include <iostream>  // Replaced <stdio.h>
#include <fstream>   
#include <sstream>   
using namespace sf;


const int M = 25;
const int N = 40;

int grid[M][N] = {0};
int ts = 18; // tile size

// Game states
const int MENU = 0;
const int GAME = 1;
const int END = 2;
const int SELECT_LEVEL = 3;
const int SCOREBOARD = 4;
int currentState = MENU;

// Difficulty modes
const int EASY = 0;
const int MEDIUM = 1;
const int HARD = 2;
const int CONTINUOUS = 3;
int difficulty = EASY;

// Game mode
const int SINGLE_PLAYER = 0;
const int TWO_PLAYER = 1;
int gameMode = SINGLE_PLAYER;

struct Enemy {
    int x, y, dx, dy;
    bool paused;
    int pattern; // 0: linear, 1: zig-zag, 2: circular
    float patternTimer;
    Enemy() {
        x = y = 300;
        dx = 4 - rand() % 8;
        dy = 4 - rand() % 8;
        paused = false;
        pattern = 0;
        patternTimer = 0;
    }
    void move(float time, float speedMultiplier) {
        if (paused) return;
        if (pattern == 0) { // Linear
            x += dx * speedMultiplier;
            if (x <= ts || x >= (N - 1) * ts || grid[y / ts][x / ts] == 1 || grid[y / ts][x / ts] == 3) {
                dx = -dx;
                x += dx * speedMultiplier;
            }
            y += dy * speedMultiplier;
            if (y <= ts || y >= (M - 1) * ts || grid[y / ts][x / ts] == 1 || grid[y / ts][x / ts] == 3) {
                dy = -dy;
                y += dy * speedMultiplier;
            }
        } else if (pattern == 1) { // Zig-zag
            patternTimer += time;
            x += dx * speedMultiplier;
            if (x <= ts || x >= (N - 1) * ts || grid[y / ts][x / ts] == 1 || grid[y / ts][x / ts] == 3) {
                dx = -dx;
                x += dx * speedMultiplier;
            }
            if (patternTimer >= 0.5f) {
                dy = -dy;
                patternTimer = 0;
            }
            y += dy * speedMultiplier;
            if (y <= ts || y >= (M - 1) * ts || grid[y / ts][x / ts] == 1 || grid[y / ts][x / ts] == 3) {
                dy = -dy;
                y += dy * speedMultiplier;
            }
        } else if (pattern == 2) { // Circular
            patternTimer += time;
            float radius = 50.0f;
            float speed = 2.0f;
            int newX = 300 + radius * cos(patternTimer * speed);
            int newY = 300 + radius * sin(patternTimer * speed);
            if (newX > ts && newX < (N - 1) * ts && newY > ts && newY < (M - 1) * ts && grid[newY / ts][newX / ts] != 1 && grid[newY / ts][newX / ts] != 3) {
                x = newX;
                y = newY;
            }
        }
    }
};

// Struct for Single-Player Mode
struct SinglePlayer {
    bool alive;
    int x, y, dx, dy;
    int score;
    int moveCount;
    int powerUpCount;
    bool powerUpActive;
    float powerUpTimer;
    int bonusCount;
    int nextPowerUpThreshold;
    bool isBuilding;
    float timer;

    SinglePlayer() {
        alive = true;
        x = 10; y = 0;
        dx = dy = 0;
        score = 0;
        moveCount = 0;
        powerUpCount = 0;
        powerUpActive = false;
        powerUpTimer = 0;
        bonusCount = 0;
        nextPowerUpThreshold = 50;
        isBuilding = false;
        timer = 0;
    }

    void reset() {
        alive = true;
        x = 10; y = 0;
        dx = dy = 0;
        score = 0;
        moveCount = 0;
        powerUpCount = 0;
        powerUpActive = false;
        powerUpTimer = 0;
        bonusCount = 0;
        nextPowerUpThreshold = 50;
        isBuilding = false;
        timer = 0;
    }
};

// Struct for Two-Player Mode
struct TwoPlayer {
    bool alive1, alive2;
    int x1, y1, dx1, dy1;
    int x2, y2, dx2, dy2;
    int score1, score2;
    int moveCount1, moveCount2;
    int powerUpCount1, powerUpCount2;
    bool powerUpActive;
    int powerOwner; // 0: none, 1: Player 1, 2: Player 2
    float powerUpTimer;
    int bonusCount1, bonusCount2;
    int nextPowerUpThreshold1, nextPowerUpThreshold2;
    bool isBuilding1, isBuilding2;
    float timer;

    TwoPlayer() {
        alive1 = true;
        alive2 = true;
        x1 = 10; y1 = 0;
        x2 = N - 11; y2 = 0;
        dx1 = dy1 = 0;
        dx2 = dy2 = 0;
        score1 = score2 = 0;
        moveCount1 = moveCount2 = 0;
        powerUpCount1 = powerUpCount2 = 0;
        powerUpActive = false;
        powerOwner = 0;
        powerUpTimer = 0;
        bonusCount1 = bonusCount2 = 0;
        nextPowerUpThreshold1 = nextPowerUpThreshold2 = 50;
        isBuilding1 = isBuilding2 = false;
        timer = 0;
    }

    void reset() {
        alive1 = true;
        alive2 = true;
        x1 = 10; y1 = 0;
        x2 = N - 11; y2 = 0;
        dx1 = dy1 = 0;
        dx2 = dy2 = 0;
        score1 = score2 = 0;
        moveCount1 = moveCount2 = 0;
        powerUpCount1 = powerUpCount2 = 0;
        powerUpActive = false;
        powerOwner = 0;
        powerUpTimer = 0;
        bonusCount1 = bonusCount2 = 0;
        nextPowerUpThreshold1 = nextPowerUpThreshold2 = 50;
        isBuilding1 = isBuilding2 = false;
        timer = 0;
    }
};

// Geometric pattern functions
void applyZigZag(Enemy& e) {
    e.pattern = 1; // Set to zig-zag
}

void applyCircular(Enemy& e) {
    e.pattern = 2; // Set to circular
}

// Separate drop functions for each player
void drop(int y, int x) { // For Player 1
    if (y < 0 || y >= M || x < 0 || x >= N || grid[y][x] != 0) return;
    grid[y][x] = -1;
    drop(y - 1, x);
    drop(y + 1, x);
    drop(y, x - 1);
    drop(y, x + 1);
}

void drop2(int y, int x) { // For Player 2
    if (y < 0 || y >= M || x < 0 || x >= N || grid[y][x] != 0) return;
    grid[y][x] = -3;
    drop2(y - 1, x);
    drop2(y + 1, x);
    drop2(y, x - 1);
    drop2(y, x + 1);
}

// Function to clear a player's trails
void clearTrails(int trailValue) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] == trailValue) {
                grid[i][j] = 0;
            }
        }
    }
    std::cout << "Cleared trails with value " << trailValue << "\n";
}

// Scoreboard functions
struct ScoreEntry {
    int score;
    int time;
};

void readScores(ScoreEntry scores[], int& count) {
    count = 0;
    std::ifstream file("scores.txt");
    if (file) {
        while (count < 5) {
            int s, t;
            file >> s >> t;
            if (file) {
                scores[count].score = s;
                scores[count].time = t;
                count++;
                std::cout << "Read score entry " << count << ": score=" << s << ", time=" << t << "\n";
            } else {
                break;
            }
        }
        file.close();
        std::cout << "Read " << count << " scores from scores.txt\n";
    } else {
        std::cout << "Error: Could not open scores.txt for reading\n";
    }
}

void writeScores(ScoreEntry scores[], int count) {
    std::ofstream file("scores.txt");
    if (file) {
        std::cout << "Writing scores to scores.txt:\n";
        for (int i = 0; i < count && i < 5; i++) {
            file << scores[i].score << " " << scores[i].time << "\n";
            std::cout << "Score " << (i + 1) << ": " << scores[i].score << ", Time: " << scores[i].time << "\n";
        }
        file.close();
        std::cout << "Scores written to scores.txt: " << (count < 5 ? count : 5) << " entries\n";
    } else {
        std::cout << "Error: Could not open scores.txt for writing\n";
    }
}

void updateScores(int newScore, int newTime) {
    ScoreEntry scores[6];
    int count;
    readScores(scores, count);
    scores[count].score = newScore;
    scores[count].time = newTime;
    count++; // Increment count to include the new score (up to 6)

    // Sort all scores (including the new one) in descending order by score, then ascending by time
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (scores[i].score < scores[j].score || 
                (scores[i].score == scores[j].score && scores[i].time > scores[j].time)) {
                ScoreEntry temp = scores[i];
                scores[i] = scores[j];
                scores[j] = temp;
            }
        }
    }

    // Write the top 5 scores back to the file
    writeScores(scores, count);
}

bool isHighScore(int newScore) {
    ScoreEntry scores[5];
    int count;
    readScores(scores, count);
    if (count < 5) return true;
    for (int i = 0; i < count; i++) {
        if (newScore > scores[i].score) return true;
    }
    return false;
}

int main() {
    srand(time(0));

    // Test leaderboard writing at startup
    std::cout << "Testing leaderboard file writing...\n";
    std::cout << "Check if scores.txt contains '999 999'\n";

    RenderWindow window(VideoMode(N * ts, M * ts), "Xonix Game!");
    window.setFramerateLimit(60);

    // Load textures
    Texture t1, t2, t3;
    t1.loadFromFile("images/tiles.png");
    t2.loadFromFile("images/gameover.png");
    t3.loadFromFile("images/enemy.png");

    Sprite sTile(t1), sGameover(t2), sEnemy(t3);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);

    // Load background images
    Texture menuBackgroundTexture, gameBackgroundTexture;
    if (!menuBackgroundTexture.loadFromFile("images/menubackground.jpg")) {
        std::cout << "Failed to load menubackground.jpg\n";
        return -1;
    }
    if (!gameBackgroundTexture.loadFromFile("images/gamebackground.jpg")) {
        std::cout << "Failed to load gamebackground.jpg\n";
        return -1;
    }

    Sprite menuBackgroundSprite, gameBackgroundSprite;
    menuBackgroundSprite.setTexture(menuBackgroundTexture);
    gameBackgroundSprite.setTexture(gameBackgroundTexture);

    // Scale backgrounds to fit window (720x450)
    menuBackgroundSprite.setScale(
        static_cast<float>(N * ts) / menuBackgroundTexture.getSize().x,
        static_cast<float>(M * ts) / menuBackgroundTexture.getSize().y
    );
    gameBackgroundSprite.setScale(
        static_cast<float>(N * ts) / gameBackgroundTexture.getSize().x,
        static_cast<float>(M * ts) / gameBackgroundTexture.getSize().y
    );

    // Load font
    Font font;
    if (!font.loadFromFile("PixelGameFont.ttf")) {
        return -1;
    }

    // Load background music
    Music menuMusic;
    if (!menuMusic.openFromFile("audio/startmenu.wav")) {
        std::cout << "Failed to load audio/startmenu.wav\n";
        return -1;
    }
    menuMusic.setLoop(true); // Set to loop for continuous playback

    Music gameMusic;
    if (!gameMusic.openFromFile("audio/gamesong.wav")) {
        std::cout << "Failed to load audio/gamesong.wav\n";
        return -1;
    }
    gameMusic.setLoop(true); // Set to loop for continuous playback

    // Start Menu
    Text menuOptions[4];
    float menuBounds[4][4];
    std::string options[] = {"Single Player", "Two Player", "Select Level", "Scoreboard"};
    for (int i = 0; i < 4; i++) {
        menuOptions[i].setFont(font);
        menuOptions[i].setString(options[i]);
        menuOptions[i].setCharacterSize(24);
        menuOptions[i].setFillColor(Color::White);
        float x = N * ts / 2 - 100;
        float y = M * ts / 2 - 80 + i * 40;
        menuOptions[i].setPosition(x, y);
        menuBounds[i][0] = x - 10;
        menuBounds[i][1] = y - 10;
        menuBounds[i][2] = 220;
        menuBounds[i][3] = 40;
    }

    // Select Level Menu
    Text levelOptions[5];
    float levelBounds[5][4];
    std::string levelText[] = {"Easy", "Medium", "Hard", "Continuous", "Back"};
    for (int i = 0; i < 5; i++) {
        levelOptions[i].setFont(font);
        levelOptions[i].setString(levelText[i]);
        levelOptions[i].setCharacterSize(24);
        levelOptions[i].setFillColor(Color::White);
        float x = N * ts / 2 - 100;
        float y = M * ts / 2 - 80 + i * 40;
        levelOptions[i].setPosition(x, y);
        levelBounds[i][0] = x - 10;
        levelBounds[i][1] = y - 10;
        levelBounds[i][2] = 220;
        levelBounds[i][3] = 40;
    }

    // End Menu
    Text endOptions[4];
    float endBounds[4][4];
    std::string endText[] = {"Game Over", "Restart", "Main Menu", "Exit Game"};
    for (int i = 0; i < 4; i++) {
        endOptions[i].setFont(font);
        endOptions[i].setString(endText[i]);
        endOptions[i].setCharacterSize(24);
        endOptions[i].setFillColor(Color::White);
        float x = N * ts / 2 - 100;
        float y = M * ts / 2 - 80 + i * 40;
        endOptions[i].setPosition(x, y);
        endBounds[i][0] = x - 10;
        endBounds[i][1] = y - 10;
        endBounds[i][2] = 220;
        endBounds[i][3] = 40;
    }
    Text scoreText, scoreText2;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(N * ts / 2 - 100, M * ts / 2 - 120);
    scoreText2.setFont(font);
    scoreText2.setCharacterSize(24);
    scoreText2.setFillColor(Color::White);
    scoreText2.setPosition(N * ts / 2 - 100, M * ts / 2 - 160);
    Text highScoreText;
    highScoreText.setFont(font);
    highScoreText.setCharacterSize(24);
    highScoreText.setFillColor(Color::Yellow);
    highScoreText.setPosition(N * ts / 2 - 100, M * ts / 2 - 200);

    // Scoreboard display
    Text scoreboardEntries[6];
    float scoreboardBounds[1][4];
    for (int i = 0; i < 5; i++) {
        scoreboardEntries[i].setFont(font);
        scoreboardEntries[i].setCharacterSize(20);
        scoreboardEntries[i].setFillColor(Color::White);
        scoreboardEntries[i].setPosition(N * ts / 2 - 100, M * ts / 2 - 100 + i * 30);
    }
    scoreboardEntries[5].setFont(font);
    scoreboardEntries[5].setString("Back");
    scoreboardEntries[5].setCharacterSize(24);
    scoreboardEntries[5].setFillColor(Color::White);
    scoreboardEntries[5].setPosition(N * ts / 2 - 100, M * ts / 2 + 50);
    scoreboardBounds[0][0] = N * ts / 2 - 110;
    scoreboardBounds[0][1] = M * ts / 2 + 40;
    scoreboardBounds[0][2] = 220;
    scoreboardBounds[0][3] = 40;

    // In-game displays
    Text gameScoreText, gameScoreText2, moveCountText, moveCountText2, gameTimeText, difficultyText, powerUpText, powerUpText2;
    gameScoreText.setFont(font);
    gameScoreText.setCharacterSize(20);
    gameScoreText.setFillColor(Color::White);
    gameScoreText.setPosition(10, 10);
    gameScoreText2.setFont(font);
    gameScoreText2.setCharacterSize(20);
    gameScoreText2.setFillColor(Color::White);
    gameScoreText2.setPosition(N * ts - 150, 10);
    moveCountText.setFont(font);
    moveCountText.setCharacterSize(20);
    moveCountText.setFillColor(Color::White);
    moveCountText.setPosition(10, 40);
    moveCountText2.setFont(font);
    moveCountText2.setCharacterSize(20);
    moveCountText2.setFillColor(Color::White);
    moveCountText2.setPosition(N * ts - 150, 40);
    gameTimeText.setFont(font);
    gameTimeText.setCharacterSize(20);
    gameTimeText.setFillColor(Color::White);
    gameTimeText.setPosition(10, 70);
    difficultyText.setFont(font);
    difficultyText.setCharacterSize(20);
    difficultyText.setFillColor(Color::White);
    difficultyText.setPosition(10, 100);
    powerUpText.setFont(font);
    powerUpText.setCharacterSize(20);
    powerUpText.setFillColor(Color::White);
    powerUpText.setPosition(10, 130);
    powerUpText2.setFont(font);
    powerUpText2.setCharacterSize(20);
    powerUpText2.setFillColor(Color::White);
    powerUpText2.setPosition(N * ts - 150, 70);

    int enemyCount = 4;
    Enemy a[10];
    float enemyTimer = 0;
    float speedTimer = 0;
    bool patternsAssigned = false;
    float speedMultiplier = 1.0f;
    bool Game = true;
    SinglePlayer sp;
    TwoPlayer tp;
    int gameTime = 0;
    float delay = 0.07;
    Clock clock, gameClock;

    // Initialize grid and log initial state
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            grid[i][j] = (i == 0 || j == 0 || i == M - 1 || j == N - 1) ? 1 : 0;
    std::cout << "Grid Initialized: grid[0][29]=" << grid[0][29] << ", grid[1][29]=" << grid[1][29] << ", grid[2][29]=" << grid[2][29] << "\n";

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                Vector2i mousePos = Mouse::getPosition(window);
                if (currentState == MENU) {
                    for (int i = 0; i < 4; i++) {
                        float bx = menuBounds[i][0];
                        float by = menuBounds[i][1];
                        float bw = menuBounds[i][2];
                        float bh = menuBounds[i][3];
                        if (mousePos.x >= bx && mousePos.x <= bx + bw &&
                            mousePos.y >= by && mousePos.y <= by + bh) {
                            if (i == 0) { // Single Player
                                gameMode = SINGLE_PLAYER;
                                currentState = GAME;
                                Game = true;
                                sp.reset();
                                gameTime = 0;
                                enemyTimer = 0;
                                speedTimer = 0;
                                speedMultiplier = 1.0f;
                                patternsAssigned = false;
                                gameClock.restart();
                                enemyCount = (difficulty == EASY) ? 2 : (difficulty == MEDIUM) ? 4 : (difficulty == HARD) ? 6 : 2;
                                for (int i = 1; i < M - 1; i++)
                                    for (int j = 1; j < N - 1; j++)
                                        grid[i][j] = 0;
                                for (int i = 0; i < enemyCount; i++) {
                                    a[i].x = a[i].y = 300;
                                    a[i].dx = 4 - rand() % 8;
                                    a[i].dy = 4 - rand() % 8;
                                    a[i].paused = false;
                                    a[i].pattern = 0;
                                    a[i].patternTimer = 0;
                                }
                                std::cout << "Single Player Started, Grid: grid[0][29]=" << grid[0][29] << ", grid[1][29]=" << grid[1][29] << ", grid[2][29]=" << grid[2][29] << "\n";
                                // Transition from MENU to GAME: Stop menu music, start game music
                                menuMusic.stop();
                                gameMusic.play();
                            } else if (i == 1) { // Two Player
                                gameMode = TWO_PLAYER;
                                currentState = GAME;
                                Game = true;
                                tp.reset();
                                gameTime = 0;
                                enemyTimer = 0;
                                speedTimer = 0;
                                speedMultiplier = 1.0f;
                                patternsAssigned = false;
                                gameClock.restart();
                                enemyCount = (difficulty == EASY) ? 2 : (difficulty == MEDIUM) ? 4 : (difficulty == HARD) ? 6 : 2;
                                for (int i = 1; i < M - 1; i++)
                                    for (int j = 1; j < N - 1; j++)
                                        grid[i][j] = 0;
                                // Clear player paths
                                for (int i = 0; i < M; i++) {
                                    grid[i][10] = (i == 0 || i == M - 1) ? 1 : 0;
                                    grid[i][N - 11] = (i == 0 || i == M - 1) ? 1 : 0;
                                }
                                for (int i = 0; i < enemyCount; i++) {
                                    a[i].x = a[i].y = 300;
                                    a[i].dx = 4 - rand() % 8;
                                    a[i].dy = 4 - rand() % 8;
                                    a[i].paused = false;
                                    a[i].pattern = 0;
                                    a[i].patternTimer = 0;
                                }
                                std::cout << "Two Player Started, Grid: grid[0][29]=" << grid[0][29] << ", grid[1][29]=" << grid[1][29] << ", grid[2][29]=" << grid[2][29] << "\n";
                                // Transition from MENU to GAME: Stop menu music, start game music
                                menuMusic.stop();
                                gameMusic.play();
                            } else if (i == 2) { // Select Level
                                currentState = SELECT_LEVEL;
                                // Transition from MENU to SELECT_LEVEL: Stop menu music
                                menuMusic.stop();
                            } else if (i == 3) { // Scoreboard
                                currentState = SCOREBOARD;
                                // Transition from MENU to SCOREBOARD: Stop menu music
                                menuMusic.stop();
                            }
                        }
                    }
                } else if (currentState == SELECT_LEVEL) {
                    for (int i = 0; i < 5; i++) {
                        float bx = levelBounds[i][0];
                        float by = levelBounds[i][1];
                        float bw = levelBounds[i][2];
                        float bh = levelBounds[i][3];
                        if (mousePos.x >= bx && mousePos.x <= bx + bw &&
                            mousePos.y >= by && mousePos.y <= by + bh) {
                            if (i == 0) difficulty = EASY;
                            else if (i == 1) difficulty = MEDIUM;
                            else if (i == 2) difficulty = HARD;
                            else if (i == 3) difficulty = CONTINUOUS;
                            currentState = MENU;
                            // Transition from SELECT_LEVEL to MENU: Start menu music
                            menuMusic.play();
                        }
                    }
                } else if (currentState == END) {
                    for (int i = 1; i < 4; i++) {
                        float bx = endBounds[i][0];
                        float by = endBounds[i][1];
                        float bw = endBounds[i][2];
                        float bh = endBounds[i][3];
                        if (mousePos.x >= bx && mousePos.x <= bx + bw &&
                            mousePos.y >= by && mousePos.y <= by + bh) {
                            if (i == 1) { // Restart
                                currentState = GAME;
                                Game = true;
                                if (gameMode == SINGLE_PLAYER) {
                                    sp.reset();
                                } else {
                                    tp.reset();
                                    // Clear player paths
                                    for (int i = 0; i < M; i++) {
                                        grid[i][10] = (i == 0 || i == M - 1) ? 1 : 0;
                                        grid[i][N - 11] = (i == 0 || i == M - 1) ? 1 : 0;
                                    }
                                }
                                gameTime = 0;
                                enemyTimer = 0;
                                speedTimer = 0;
                                speedMultiplier = 1.0f;
                                patternsAssigned = false;
                                gameClock.restart();
                                enemyCount = (difficulty == EASY) ? 2 : (difficulty == MEDIUM) ? 4 : (difficulty == HARD) ? 6 : 2;
                                for (int i = 1; i < M - 1; i++)
                                    for (int j = 1; j < N - 1; j++)
                                        grid[i][j] = 0;
                                for (int i = 0; i < enemyCount; i++) {
                                    a[i].x = a[i].y = 300;
                                    a[i].dx = 4 - rand() % 8;
                                    a[i].dy = 4 - rand() % 8;
                                    a[i].paused = false;
                                    a[i].pattern = 0;
                                    a[i].patternTimer = 0;
                                }
                                std::cout << "Game Restarted: Mode=" << (gameMode == SINGLE_PLAYER ? "Single" : "Two") 
                                          << ", Grid: grid[0][29]=" << grid[0][29] << ", grid[1][29]=" << grid[1][29] 
                                          << ", grid[2][29]=" << grid[2][29] << "\n";
                                // Transition from END to GAME: Start game music
                                gameMusic.play();
                            } else if (i == 2) { // Main Menu
                                currentState = MENU;
                                // Transition from END to MENU: Start menu music
                                menuMusic.play();
                            } else if (i == 3) { // Exit Game
                                window.close();
                            }
                        }
                    }
                } else if (currentState == SCOREBOARD) {
                    float bx = scoreboardBounds[0][0];
                    float by = scoreboardBounds[0][1];
                    float bw = scoreboardBounds[0][2];
                    float bh = scoreboardBounds[0][3];
                    if (mousePos.x >= bx && mousePos.x <= bx + bw &&
                        mousePos.y >= by && mousePos.y <= by + bh) {
                        currentState = MENU;
                        // Transition from SCOREBOARD to MENU: Start menu music
                        menuMusic.play();
                    }
                }
            }

            if (currentState == GAME && e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Escape) {
                    currentState = MENU;
                    // Transition from GAME to MENU: Stop game music, start menu music
                    gameMusic.stop();
                    menuMusic.play();
                } else if (e.key.code == Keyboard::Space) {
                    if (gameMode == SINGLE_PLAYER && sp.powerUpCount > 0 && !sp.powerUpActive) {
                        sp.powerUpActive = true;
                        sp.powerUpTimer = 0;
                        sp.powerUpCount--;
                        for (int i = 0; i < enemyCount; i++) a[i].paused = true;
                    } else if (gameMode == TWO_PLAYER && tp.powerUpCount1 > 0 && !tp.powerUpActive) {
                        tp.powerUpActive = true;
                        tp.powerUpTimer = 0;
                        tp.powerUpCount1--;
                        tp.powerOwner = 1;
                        for (int i = 0; i < enemyCount; i++) a[i].paused = true;
                        std::cout << "P1 Power-Up Activated, P2 Frozen\n";
                    }
                } else if (e.key.code == Keyboard::G && gameMode == TWO_PLAYER && tp.powerUpCount2 > 0 && !tp.powerUpActive) {
                    tp.powerUpActive = true;
                    tp.powerUpTimer = 0;
                    tp.powerUpCount2--;
                    tp.powerOwner = 2;
                    for (int i = 0; i < enemyCount; i++) a[i].paused = true;
                    std::cout << "P2 Power-Up Activated, P1 Frozen\n";
                }
            }
        }

        if (currentState == MENU) {
            window.clear();
            window.draw(menuBackgroundSprite); // Draw menu background
            for (int i = 0; i < 4; i++) window.draw(menuOptions[i]);
            window.display();
            // Ensure menu music is playing
            if (menuMusic.getStatus() != Music::Playing) {
                gameMusic.stop(); // Stop game music if it's playing
                menuMusic.play();
            }
            continue;
        } else if (currentState == SELECT_LEVEL) {
            window.clear();
            window.draw(menuBackgroundSprite); // Draw menu background
            for (int i = 0; i < 5; i++) window.draw(levelOptions[i]);
            window.display();
            // No music in SELECT_LEVEL state
            menuMusic.stop();
            gameMusic.stop();
            continue;
        } else if (currentState == SCOREBOARD) {
            window.clear();
            window.draw(menuBackgroundSprite); // Draw menu background
            ScoreEntry scores[5];
            int count;
            readScores(scores, count);
            for (int i = 0; i < 5; i++) {
                std::stringstream entry;
                if (i < count) {
                    entry << (i + 1) << ". Score: " << scores[i].score << ", Time: " << scores[i].time << "s";
                } else {
                    entry << (i + 1) << ". ---";
                }
                scoreboardEntries[i].setString(entry.str());
                window.draw(scoreboardEntries[i]);
            }
            window.draw(scoreboardEntries[5]);
            window.display();
            // No music in SCOREBOARD state
            menuMusic.stop();
            gameMusic.stop();
            continue;
        }

        if (currentState == GAME) {
            float time = clock.getElapsedTime().asSeconds();
            clock.restart();
            if (gameMode == SINGLE_PLAYER) sp.timer += time;
            else tp.timer += time;
            enemyTimer += time;
            speedTimer += time;

            if (gameMode == SINGLE_PLAYER) {
                if (sp.powerUpActive) {
                    sp.powerUpTimer += time;
                    if (sp.powerUpTimer >= 3.0f) {
                        sp.powerUpActive = false;
                        for (int i = 0; i < enemyCount; i++) a[i].paused = false;
                    }
                }
            } else {
                if (tp.powerUpActive) {
                    tp.powerUpTimer += time;
                    if (tp.powerUpTimer >= 3.0f) {
                        tp.powerUpActive = false;
                        tp.powerOwner = 0;
                        for (int i = 0; i < enemyCount; i++) a[i].paused = false;
                        if (tp.powerOwner == 1) {
                            tp.alive2 = true;
                            // Removed position reset: tp.x2 = N - 11; tp.y2 = 0;
                            tp.dx2 = tp.dy2 = 0;
                            tp.isBuilding2 = false;
                            // Do not clear P2's path since the player resumes from their current position
                            std::cout << "P2 Resumed: x2=" << tp.x2 << ", y2=" << tp.y2 
                                      << ", alive=" << tp.alive2 << ", isBuilding2=" << tp.isBuilding2 << "\n";
                            std::cout << "P1 Power-Up Ended\n";
                        } else if (tp.powerOwner == 2) {
                            tp.alive1 = true;
                            // Removed position reset: tp.x1 = 10; tp.y1 = 0;
                            tp.dx1 = tp.dy1 = 0;
                            tp.isBuilding1 = false;
                            // Do not clear P1's path since the player resumes from their current position
                            std::cout << "P1 Resumed: x1=" << tp.x1 << ", y1=" << tp.y1 
                                      << ", alive=" << tp.alive1 << ", isBuilding1=" << tp.isBuilding1 << "\n";
                            std::cout << "P2 Power-Up Ended\n";
                        }
                    }
                }
            }
            gameTime = static_cast<int>(gameClock.getElapsedTime().asSeconds());

            if (difficulty == CONTINUOUS && enemyTimer >= 20.0f && enemyCount < 10) {
                enemyCount += 2;
                if (enemyCount > 10) enemyCount = 10;
                for (int i = enemyCount - 2; i < enemyCount; i++) {
                    a[i].x = a[i].y = 300;
                    a[i].dx = (4 - rand() % 8) * speedMultiplier;
                    a[i].dy = (4 - rand() % 8) * speedMultiplier;
                    a[i].paused = (gameMode == SINGLE_PLAYER) ? sp.powerUpActive : tp.powerUpActive;
                    a[i].pattern = 0;
                    a[i].patternTimer = 0;
                }
                enemyTimer = 0;
                std::cout << "Added enemies: count=" << enemyCount << "\n";
            }

            if (speedTimer >= 20.0f) {
                speedMultiplier *= 1.2f;
                for (int i = 0; i < enemyCount; i++) {
                    if (a[i].pattern == 0) {
                        a[i].dx = (a[i].dx < 0 ? -4 : 4) * speedMultiplier;
                        a[i].dy = (a[i].dy < 0 ? -4 : 4) * speedMultiplier;
                    }
                }
                speedTimer = 0;
                std::cout << "Speed increased: multiplier=" << speedMultiplier << "\n";
            }

            if (gameTime >= 30 && !patternsAssigned) {
                patternsAssigned = true;
                int half = (enemyCount + 1) / 2; // First half for zig-zag
                for (int i = 0; i < half; i++) {
                    applyZigZag(a[i]); // First half zig-zag
                }
                for (int i = half; i < enemyCount; i++) {
                    applyCircular(a[i]); // Second half circular
                }
                std::cout << "Patterns assigned: " << half << " zig-zag, " << (enemyCount - half) << " circular\n";
            }

            if (gameMode == SINGLE_PLAYER) {
                if (sp.alive) {
                    if (Keyboard::isKeyPressed(Keyboard::Left)) { sp.dx = -1; sp.dy = 0; }
                    if (Keyboard::isKeyPressed(Keyboard::Right)) { sp.dx = 1; sp.dy = 0; }
                    if (Keyboard::isKeyPressed(Keyboard::Up)) { sp.dx = 0; sp.dy = -1; }
                    if (Keyboard::isKeyPressed(Keyboard::Down)) { sp.dx = 0; sp.dy = 1; }
                }
            } else {
                if (tp.alive1) {
                    if (Keyboard::isKeyPressed(Keyboard::Left)) { tp.dx1 = -1; tp.dy1 = 0; }
                    if (Keyboard::isKeyPressed(Keyboard::Right)) { tp.dx1 = 1; tp.dy1 = 0; }
                    if (Keyboard::isKeyPressed(Keyboard::Up)) { tp.dx1 = 0; tp.dy1 = -1; }
                    if (Keyboard::isKeyPressed(Keyboard::Down)) { tp.dx1 = 0; tp.dy1 = 1; }
                }
                if (tp.alive2) {
                    if (Keyboard::isKeyPressed(Keyboard::W)) { tp.dx2 = 0; tp.dy2 = -1; }
                    if (Keyboard::isKeyPressed(Keyboard::S)) { tp.dx2 = 0; tp.dy2 = 1; }
                    if (Keyboard::isKeyPressed(Keyboard::A)) { tp.dx2 = -1; tp.dy2 = 0; }
                    if (Keyboard::isKeyPressed(Keyboard::D)) { tp.dx2 = 1; tp.dy2 = 0; }
                }
            }

            if (!Game || (gameMode == SINGLE_PLAYER && !sp.alive) || (gameMode == TWO_PLAYER && !tp.alive1 && !tp.alive2)) {
                if (gameMode == SINGLE_PLAYER) {
                    std::cout << "Updating scores for Single Player: score=" << sp.score << ", time=" << gameTime << "\n";
                    updateScores(sp.score, gameTime);
                } else {
                    int finalScore = tp.score1 > tp.score2 ? tp.score1 : tp.score2;
                    std::cout << "Updating scores for Two Player: score=" << finalScore << ", time=" << gameTime << "\n";
                    updateScores(finalScore, gameTime);
                }
                currentState = END;
                std::cout << "Game Over: P1=" << (gameMode == SINGLE_PLAYER ? sp.alive : tp.alive1) 
                          << ", P2=" << (gameMode == SINGLE_PLAYER ? 0 : tp.alive2) << "\n";
                // Transition from GAME to END: Stop game music
                gameMusic.stop();
                continue;
            }

            if ((gameMode == SINGLE_PLAYER && sp.timer > delay) || (gameMode == TWO_PLAYER && tp.timer > delay)) {
                if (gameMode == SINGLE_PLAYER) {
                    if (sp.alive) {
                        int newX = sp.x + sp.dx;
                        int newY = sp.y + sp.dy;
                        std::cout << "P1 Attempt Move: x=" << sp.x << ", y=" << sp.y << ", dx=" << sp.dx << ", dy=" << sp.dy 
                                  << ", timer=" << sp.timer << ", grid[" << newY << "][" << newX << "]=" 
                                  << (newY >= 0 && newY < M && newX >= 0 && newX < N ? grid[newY][newX] : -1) << "\n";
                        if (newX >= 0 && newX < N && newY >= 0 && newY < M) {
                            if (grid[newY][newX] == 2 && !(newX == sp.x && newY == sp.y)) {
                                sp.alive = false;
                                std::cout << "P1 Died: Hit path at x=" << newX << ", y=" << newY << ", grid=" << grid[newY][newX] << "\n";
                            } else {
                                sp.x = newX;
                                sp.y = newY;
                                std::cout << "P1 Moved: x=" << sp.x << ", y=" << sp.y << ", grid=" << grid[sp.y][sp.x] 
                                          << ", isBuilding=" << sp.isBuilding << ", alive=" << sp.alive << "\n";
                                if (grid[sp.y][sp.x] == 0) {
                                    if (!sp.isBuilding) {
                                        sp.moveCount++;
                                        sp.isBuilding = true;
                                        std::cout << "P1 Building: moveCount=" << sp.moveCount << "\n";
                                    }
                                    grid[sp.y][sp.x] = 2;
                                }
                                if (grid[sp.y][sp.x] == 1 || grid[sp.y][sp.x] == 3) {
                                    sp.isBuilding = false;
                                    std::cout << "P1 Stopped: Hit border at x=" << sp.x << ", y=" << sp.y << "\n";
                                }
                            }
                        }
                    }
                } else {
                    if (tp.alive1 && (!tp.powerUpActive || tp.powerOwner == 1)) {
                        int newX1 = tp.x1 + tp.dx1;
                        int newY1 = tp.y1 + tp.dy1;
                        std::cout << "P1 Attempt Move: x1=" << tp.x1 << ", y1=" << tp.y1 << ", dx1=" << tp.dx1 << ", dy1=" << tp.dy1 
                                  << ", timer=" << tp.timer << ", grid[" << newY1 << "][" << newX1 << "]=" 
                                  << (newY1 >= 0 && newY1 < M && newX1 >= 0 && newX1 < N ? grid[newY1][newX1] : -1) << "\n";
                        if (newX1 >= 0 && newX1 < N && newY1 >= 0 && newY1 < M) {
                            if ((grid[newY1][newX1] == 2 || grid[newY1][newX1] == -2) && !(newX1 == tp.x1 && newY1 == tp.y1)) {
                                tp.alive1 = false;
                                clearTrails(2); // Clear Player 1's trails
                                std::cout << "P1 Died: Hit path at x1=" << newX1 << ", y1=" << newY1 << ", grid=" << grid[newY1][newX1] << "\n";
                            } else {
                                tp.x1 = newX1;
                                tp.y1 = newY1;
                                std::cout << "P1 Moved: x1=" << tp.x1 << ", y1=" << tp.y1 << ", grid=" << grid[tp.y1][tp.x1] 
                                          << ", isBuilding1=" << tp.isBuilding1 << ", alive=" << tp.alive1 << "\n";
                                if (tp.alive2) {
                                    bool isP1Constructing = grid[tp.y1][tp.x1] == 0;
                                    bool isP2Constructing = grid[tp.y2][tp.x2] == 0;
                                    if (tp.x1 == tp.x2 && tp.y1 == tp.y2 && isP1Constructing && isP2Constructing) {
                                        tp.alive1 = false;
                                        tp.alive2 = false;
                                        clearTrails(2); // Clear Player 1's trails
                                        clearTrails(-2); // Clear Player 2's trails
                                        std::cout << "Both Died: Constructing collision at x1=" << tp.x1 << ", y1=" << tp.y1 << "\n";
                                    } else if (tp.x1 == tp.x2 && tp.y1 == tp.y2) {
                                        if (isP1Constructing && !isP2Constructing) {
                                            tp.alive1 = false;
                                            clearTrails(2); // Clear Player 1's trails
                                            std::cout << "P1 Died: Constructing hit idle P2 at x1=" << tp.x1 << ", y1=" << tp.y1 << "\n";
                                        }
                                    }
                                }
                                if (grid[tp.y1][tp.x1] == 0) {
                                    if (!tp.isBuilding1) {
                                        tp.moveCount1++;
                                        tp.isBuilding1 = true;
                                        std::cout << "P1 Building: moveCount1=" << tp.moveCount1 << "\n";
                                    }
                                    grid[tp.y1][tp.x1] = 2;
                                }
                                if (grid[tp.y1][tp.x1] == 1 || grid[tp.y1][tp.x1] == 3) {
                                    tp.isBuilding1 = false;
                                    std::cout << "P1 Stopped: Hit border at x1=" << tp.x1 << ", y1=" << tp.y1 << "\n";
                                }
                            }
                        }
                        // Keep player in bounds
                        if (tp.x1 < 0) tp.x1 = 0;
                        if (tp.x1 > N - 1) tp.x1 = N - 1;
                        if (tp.y1 < 0) tp.y1 = 0;
                        if (tp.y1 > M - 1) tp.y1 = M - 1;
                    }

                    if (tp.alive2 && (!tp.powerUpActive || tp.powerOwner == 2)) {
                        int newX2 = tp.x2 + tp.dx2;
                        int newY2 = tp.y2 + tp.dy2;
                        std::cout << "P2 Attempt Move: x2=" << tp.x2 << ", y2=" << tp.y2 << ", dx2=" << tp.dx2 << ", dy2=" << tp.dy2 
                                  << ", timer=" << tp.timer << ", grid[" << newY2 << "][" << newX2 << "]=" 
                                  << (newY2 >= 0 && newY2 < M && newX2 >= 0 && newX2 < N ? grid[newY2][newX2] : -1) << "\n";
                        if (newX2 >= 0 && newX2 < N && newY2 >= 0 && newY2 < M) {
                            if ((grid[newY2][newX2] == -2 || grid[newY2][newX2] == 2) && !(newX2 == tp.x2 && newY2 == tp.y2)) {
                                tp.alive2 = false;
                                clearTrails(-2); // Clear Player 2's trails
                                std::cout << "P2 Died: Hit path at x2=" << newX2 << ", y2=" << newY2 << ", grid=" << grid[newY2][newX2] << "\n";
                            } else {
                                tp.x2 = newX2;
                                tp.y2 = newY2;
                                std::cout << "P2 Moved: x2=" << tp.x2 << ", y2=" << tp.y2 << ", grid=" << grid[tp.y2][tp.x2] 
                                          << ", isBuilding2=" << tp.isBuilding2 << ", alive=" << tp.alive2 << "\n";
                                if (tp.alive1) {
                                    bool isP1Constructing = grid[tp.y1][tp.x1] == 0;
                                    bool isP2Constructing = grid[tp.y2][tp.x2] == 0;
                                    if (tp.x2 == tp.x1 && tp.y2 == tp.y1 && isP2Constructing && isP1Constructing) {
                                        tp.alive1 = false;
                                        tp.alive2 = false;
                                        clearTrails(2); // Clear Player 1's trails
                                        clearTrails(-2); // Clear Player 2's trails
                                        std::cout << "Both Died: Constructing collision at x2=" << tp.x2 << ", y2=" << tp.y2 << "\n";
                                    } else if (tp.x2 == tp.x1 && tp.y2 == tp.y1) {
                                        if (isP2Constructing && !isP1Constructing) {
                                            tp.alive2 = false;
                                            clearTrails(-2); // Clear Player 2's trails
                                            std::cout << "P2 Died: Constructing hit idle P1 at x2=" << tp.x2 << ", y2=" << tp.y2 << "\n";
                                        }
                                    }
                                }
                                if (grid[tp.y2][tp.x2] == 0) {
                                    if (!tp.isBuilding2) {
                                        tp.moveCount2++;
                                        tp.isBuilding2 = true;
                                        std::cout << "P2 Building: moveCount2=" << tp.moveCount2 << "\n";
                                    }
                                    grid[tp.y2][tp.x2] = -2;
                                }
                                if (grid[tp.y2][tp.x2] == 1 || grid[tp.y2][tp.x2] == 3) {
                                    tp.isBuilding2 = false;
                                    std::cout << "P2 Stopped: Hit border at x2=" << tp.x2 << ", y2=" << tp.y2 << "\n";
                                }
                            }
                        }
                        // Keep player in bounds
                        if (tp.x2 < 0) tp.x2 = 0;
                        if (tp.x2 > N - 1) tp.x2 = N - 1;
                        if (tp.y2 < 0) tp.y2 = 0;
                        if (tp.y2 > M - 1) tp.y2 = M - 1;
                    }

                    // Reset positions if dead
                    if (!tp.alive1) {
                        tp.x1 = tp.y1 = 0;
                    }
                    if (!tp.alive2) {
                        tp.x2 = tp.y2 = 0;
                    }
                }

                if (gameMode == SINGLE_PLAYER) sp.timer = 0;
                else tp.timer = 0;
            }

            if (!tp.powerUpActive) {
                for (int i = 0; i < enemyCount; i++) a[i].move(time, speedMultiplier);
            }

            if (gameMode == SINGLE_PLAYER) {
                if (sp.alive && (grid[sp.y][sp.x] == 1 || grid[sp.y][sp.x] == 3)) {
                    sp.dx = sp.dy = 0;
                    for (int i = 0; i < enemyCount; i++)
                        drop(a[i].y / ts, a[i].x / ts);
                    int capturedTiles = 0;
                    for (int i = 0; i < M; i++)
                        for (int j = 0; j < N; j++) {
                            if (grid[i][j] == -1) {
                                grid[i][j] = 0;
                            } else if (grid[i][j] == 0 || grid[i][j] == 2) {
                                grid[i][j] = 1;
                                capturedTiles++;
                            }
                        }
                    int points = capturedTiles; // 1 point per tile
                    int threshold = (sp.bonusCount >= 3) ? 5 : 10; // Threshold: 10 tiles initially, 5 after 3 successful captures
                    if (capturedTiles > threshold) {
                        points *= 2; // 2x multiplier for exceeding threshold
                        sp.bonusCount++;
                    }
                    if (sp.bonusCount >= 5 && capturedTiles > 5) {
                        points = capturedTiles * 4; // 4x multiplier if bonusCount >= 5 and capturedTiles > 5
                    }
                    sp.score += points;
                    if (sp.score >= sp.nextPowerUpThreshold) {
                        sp.powerUpCount++;
                        if (sp.nextPowerUpThreshold == 50) sp.nextPowerUpThreshold = 70;
                        else if (sp.nextPowerUpThreshold == 70) sp.nextPowerUpThreshold = 100;
                        else sp.nextPowerUpThreshold += 30;
                    }
                    std::cout << "P1 Captured: " << capturedTiles << " tiles, points=" << points 
                              << " (threshold=" << threshold << ", bonusCount=" << sp.bonusCount 
                              << "), score=" << sp.score << ", powerUpCount=" << sp.powerUpCount << "\n";
                }
            } else {
                if (tp.alive1 && (grid[tp.y1][tp.x1] == 1 || grid[tp.y1][tp.x1] == 3)) {
                    tp.dx1 = tp.dy1 = 0;
                    for (int i = 0; i < enemyCount; i++)
                        drop(a[i].y / ts, a[i].x / ts);
                    int capturedTiles = 0;
                    for (int i = 0; i < M; i++)
                        for (int j = 0; j < N; j++) {
                            if (grid[i][j] == -1) {
                                grid[i][j] = 0;
                            } else if (grid[i][j] == 0 || grid[i][j] == 2) { // Only count Player 1's trails
                                grid[i][j] = 1; // Player 1 captures as 1 (blue)
                                capturedTiles++;
                            }
                        }
                    int points = capturedTiles; // 1 point per tile
                    int threshold = (tp.bonusCount1 >= 3) ? 5 : 10; // Threshold: 10 tiles initially, 5 after 3 successful captures
                    if (capturedTiles > threshold) {
                        points *= 2; // 2x multiplier for exceeding threshold
                        tp.bonusCount1++;
                    }
                    if (tp.bonusCount1 >= 5 && capturedTiles > 5) {
                        points = capturedTiles * 4; // 4x multiplier if bonusCount >= 5 and capturedTiles > 5
                    }
                    tp.score1 += points;
                    if (tp.score1 >= tp.nextPowerUpThreshold1) {
                        tp.powerUpCount1++;
                        if (tp.nextPowerUpThreshold1 == 50) tp.nextPowerUpThreshold1 = 70;
                        else if (tp.nextPowerUpThreshold1 == 70) tp.nextPowerUpThreshold1 = 100;
                        else tp.nextPowerUpThreshold1 += 30;
                    }
                    std::cout << "P1 Captured: " << capturedTiles << " tiles, points=" << points 
                              << " (threshold=" << threshold << ", bonusCount1=" << tp.bonusCount1 
                              << "), score1=" << tp.score1 << ", powerUpCount1=" << tp.powerUpCount1 << "\n";
                }

                if (tp.alive2 && (grid[tp.y2][tp.x2] == 1 || grid[tp.y2][tp.x2] == 3)) {
                    tp.dx2 = tp.dy2 = 0;
                    for (int i = 0; i < enemyCount; i++)
                        drop2(a[i].y / ts, a[i].x / ts);
                    int capturedTiles = 0;
                    for (int i = 0; i < M; i++)
                        for (int j = 0; j < N; j++) {
                            if (grid[i][j] == -3) {
                                grid[i][j] = 0;
                            } else if (grid[i][j] == 0 || grid[i][j] == -2) { // Only count Player 2's trails
                                grid[i][j] = 3; // Player 2 captures as 3 (different color)
                                capturedTiles++;
                            }
                        }
                    int points = capturedTiles; // 1 point per tile
                    int threshold = (tp.bonusCount2 >= 3) ? 5 : 10; // Threshold: 10 tiles initially, 5 after 3 successful captures
                    if (capturedTiles > threshold) {
                        points *= 2; // 2x multiplier for exceeding threshold
                        tp.bonusCount2++;
                    }
                    if (tp.bonusCount2 >= 5 && capturedTiles > 5) {
                        points = capturedTiles * 4; // 4x multiplier if bonusCount >= 5 and capturedTiles > 5
                    }
                    tp.score2 += points;
                    if (tp.score2 >= tp.nextPowerUpThreshold2) {
                        tp.powerUpCount2++;
                        if (tp.nextPowerUpThreshold2 == 50) tp.nextPowerUpThreshold2 = 70;
                        else if (tp.nextPowerUpThreshold2 == 70) tp.nextPowerUpThreshold2 = 100;
                        else tp.nextPowerUpThreshold2 += 30;
                    }
                    std::cout << "P2 Captured: " << capturedTiles << " tiles, points=" << points 
                              << " (threshold=" << threshold << ", bonusCount2=" << tp.bonusCount2 
                              << "), score2=" << tp.score2 << ", powerUpCount2=" << tp.powerUpCount2 << "\n";
                }
            }

            for (int i = 0; i < enemyCount; i++) {
                if (gameMode == SINGLE_PLAYER) {
                    if (sp.alive && grid[a[i].y / ts][a[i].x / ts] == 2) {
                        sp.alive = false;
                        std::cout << "P1 Died: Hit by enemy\n";
                    }
                } else {
                    if (tp.alive1 && grid[a[i].y / ts][a[i].x / ts] == 2) {
                        tp.alive1 = false;
                        clearTrails(2); // Clear Player 1's trails
                        std::cout << "P1 Died: Hit by enemy\n";
                    }
                    if (tp.alive2 && grid[a[i].y / ts][a[i].x / ts] == -2) {
                        tp.alive2 = false;
                        clearTrails(-2); // Clear Player 2's trails
                        std::cout << "P2 Died: Hit by enemy\n";
                    }
                }
            }
        }

        // Draw
        window.clear();

        if (currentState == GAME) {
            window.draw(gameBackgroundSprite); // Draw game background
            for (int i = 0; i < M; i++)
                for (int j = 0; j < N; j++) {
                    if (grid[i][j] == 0) continue;
                    if (grid[i][j] == 1) sTile.setTextureRect(IntRect(0, 0, ts, ts)); // Blue blocks for P1 captures
                    else if (grid[i][j] == 2 || grid[i][j] == -2) sTile.setTextureRect(IntRect(54, 0, ts, ts)); // Green blocks for building
                    else if (grid[i][j] == 3) sTile.setTextureRect(IntRect(90, 0, ts, ts)); // Different color for P2 captures (assumed red/purple)
                    sTile.setPosition(j * ts, i * ts);
                    window.draw(sTile);
                }

            if (gameMode == SINGLE_PLAYER) {
                if (sp.alive) {
                    sTile.setTextureRect(IntRect(36, 0, ts, ts)); // Player 1 sprite (white/neutral)
                    sTile.setPosition(sp.x * ts, sp.y * ts);
                    window.draw(sTile);
                }
            } else {
                if (tp.alive1) {
                    sTile.setTextureRect(IntRect(36, 0, ts, ts)); // Player 1 sprite (white/neutral)
                    sTile.setPosition(tp.x1 * ts, tp.y1 * ts);
                    window.draw(sTile);
                }
                if (tp.alive2) {
                    sTile.setTextureRect(IntRect(72, 0, ts, ts)); // Player 2 sprite (assumed red)
                    sTile.setPosition(tp.x2 * ts, tp.y2 * ts);
                    window.draw(sTile);
                }
            }

            sEnemy.rotate(10);
            for (int i = 0; i < enemyCount; i++) {
                sEnemy.setPosition(a[i].x, a[i].y);
                window.draw(sEnemy);
            }

            std::stringstream scoreStr;
            if (gameMode == SINGLE_PLAYER) {
                scoreStr << "P1 Score: " << sp.score;
            } else {
                scoreStr << "P1 Score: " << tp.score1;
            }
            gameScoreText.setString(scoreStr.str());
            window.draw(gameScoreText);

            if (gameMode == TWO_PLAYER) {
                scoreStr.str(""); // Clear the stream
                scoreStr << "P2 Score: " << tp.score2;
                gameScoreText2.setString(scoreStr.str());
                window.draw(gameScoreText2);
            }

            std::stringstream moveStr;
            if (gameMode == SINGLE_PLAYER) {
                moveStr << "P1 Moves: " << sp.moveCount;
            } else {
                moveStr << "P1 Moves: " << tp.moveCount1;
            }
            moveCountText.setString(moveStr.str());
            window.draw(moveCountText);

            if (gameMode == TWO_PLAYER) {
                moveStr.str("");
                moveStr << "P2 Moves: " << tp.moveCount2;
                moveCountText2.setString(moveStr.str());
                window.draw(moveCountText2);
            }

            std::stringstream timeStr;
            timeStr << "Time: " << gameTime << "s";
            gameTimeText.setString(timeStr.str());
            window.draw(gameTimeText);

            std::stringstream diffStr;
            diffStr << "Mode: " << (difficulty == EASY ? "Easy" : difficulty == MEDIUM ? "Medium" : difficulty == HARD ? "Hard" : "Continuous");
            difficultyText.setString(diffStr.str());
            window.draw(difficultyText);

            std::stringstream powerStr;
            if (gameMode == SINGLE_PLAYER) {
                powerStr << "P1 Power-Ups: " << sp.powerUpCount;
            } else {
                powerStr << "P1 Power-Ups: " << tp.powerUpCount1;
            }
            powerUpText.setString(powerStr.str());
            window.draw(powerUpText);

            if (gameMode == TWO_PLAYER) {
                powerStr.str("");
                powerStr << "P2 Power-Ups: " << tp.powerUpCount2;
                powerUpText2.setString(powerStr.str());
                window.draw(powerUpText2);
            }
            // Ensure game music is playing
            if (gameMusic.getStatus() != Music::Playing) {
                menuMusic.stop(); // Stop menu music if it's playing
                gameMusic.play();
            }
        } else if (currentState == END) {
            window.draw(menuBackgroundSprite); // Draw menu background
            if (gameMode == SINGLE_PLAYER) {
                std::stringstream scoreStr;
                scoreStr << "Score: " << sp.score;
                scoreText.setString(scoreStr.str());
                window.draw(scoreText);
                if (isHighScore(sp.score)) {
                    highScoreText.setString("New High Score!");
                    window.draw(highScoreText);
                }
            } else {
                // Only display the winner message for Two-Player Mode
                std::stringstream winnerStr;
                if (tp.score1 > tp.score2) winnerStr << "Player 1 Wins!";
                else if (tp.score2 > tp.score1) winnerStr << "Player 2 Wins!";
                else winnerStr << "Tie!";
                highScoreText.setString(winnerStr.str());
                window.draw(highScoreText);
            }
            for (int i = 0; i < 4; i++) window.draw(endOptions[i]);
            // No music in END state
            menuMusic.stop();
            gameMusic.stop();
        }

        window.display();
    }

    return 0;
}
