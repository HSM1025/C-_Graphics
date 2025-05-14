#define _CRT_SECURE_NO_WARNINGS
#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>

// 2048 게임 코드 부분
#define SIZE 4
#define TILE_SIZE 100
#define TILE_PADDING 10
#define N 9
#define CELL_SIZE 60
#define BOARD_SIZE (CELL_SIZE * N)
#define SCREEN_WIDTH 700
#define SCREEN_HEIGHT 750

int board1[SIZE][SIZE] = { 0 };

void AddRandomTile() {
    int empty[SIZE * SIZE][2];
    int emptyCount = 0;

    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
            if (board1[y][x] == 0) {
                empty[emptyCount][0] = y;
                empty[emptyCount][1] = x;
                emptyCount++;
            }

    if (emptyCount > 0) {
        int index = rand() % emptyCount;
        int y = empty[index][0];
        int x = empty[index][1];
        board1[y][x] = (rand() % 10 == 0) ? 4 : 2;
    }
}

bool SlideAndMergeRow(int* row) {
    bool moved = false;
    int temp[SIZE] = { 0 };
    int idx = 0;

    for (int i = 0; i < SIZE; i++) {
        if (row[i] != 0) {
            if (temp[idx] == 0) {
                temp[idx] = row[i];
            }
            else if (temp[idx] == row[i]) {
                temp[idx++] *= 2;
                moved = true;
            }
            else {
                temp[++idx] = row[i];
            }
        }
    }

    for (int i = 0; i < SIZE; i++) {
        if (row[i] != temp[i]) {
            moved = true;
            row[i] = temp[i];
        }
    }

    return moved;
}

bool Move(int direction) {
    bool moved = false;

    for (int i = 0; i < SIZE; i++) {
        int row[SIZE];
        for (int j = 0; j < SIZE; j++) {
            if (direction == 0) row[j] = board1[i][j];          // Left
            else if (direction == 1) row[j] = board1[j][i];     // Up
            else if (direction == 2) row[j] = board1[i][SIZE - 1 - j]; // Right
            else row[j] = board1[SIZE - 1 - j][i];              // Down
        }

        bool rowMoved = SlideAndMergeRow(row);
        moved |= rowMoved;

        for (int j = 0; j < SIZE; j++) {
            if (direction == 0) board1[i][j] = row[j];
            else if (direction == 1) board1[j][i] = row[j];
            else if (direction == 2) board1[i][SIZE - 1 - j] = row[j];
            else board1[SIZE - 1 - j][i] = row[j];
        }
    }

    return moved;
}

void DrawBoard() {
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            int value = board1[y][x];
            Rectangle tile = {
                x * (TILE_SIZE + TILE_PADDING) + TILE_PADDING,
                y * (TILE_SIZE + TILE_PADDING) + TILE_PADDING,
                TILE_SIZE,
                TILE_SIZE
            };

            // 기본 색상 (빈칸)
            Color tileColor = Color{ 202, 192, 180 ,255 };

            // 값에 따라 색상 지정
            switch (value) {
            case 2:  tileColor = Color{ 236, 229, 219, 255 }; break; // 회색
            case 4:  tileColor = Color{ 235, 224, 202, 255 }; break; // 살구색
            case 8:  tileColor = Color{ 232, 180, 130, 255 }; break; // 연주황
            case 16: tileColor = Color{ 232, 154, 108, 255 }; break;   // 주황
            case 32: tileColor = Color{ 230, 131, 102, 255 }; break;  // 자홍색
            case 64: tileColor = Color{ 229, 106, 072, 255 }; break;
            default: if (value) tileColor = Color{ 234, 209, 127, 255 }; break;           // 기타 숫자
            }

            DrawRectangleRec(tile, tileColor);
            if (value < 7 && value>0) {
                DrawText(TextFormat("%d", value),
                    tile.x + TILE_SIZE / 2.5,
                    tile.y + TILE_SIZE / 2.8,
                    35,
                    Color{
                    117, 110, 100, 255
                    });
            }
            else if (value >= 128) {
                DrawText(TextFormat("%d", value),
                    tile.x + TILE_SIZE / 3.8,
                    tile.y + TILE_SIZE / 2.8,
                    35,
                    RAYWHITE);
            }
            else if (value >= 1024) {
                DrawText(TextFormat("%d", value),
                    tile.x + TILE_SIZE / 6,
                    tile.y + TILE_SIZE / 2.8,
                    35,
                    RAYWHITE);
            }
            else if (value) {
                DrawText(TextFormat("%d", value),
                    tile.x + TILE_SIZE / 2.5,
                    tile.y + TILE_SIZE / 2.8,
                    35,
                    RAYWHITE);
            }
        }
    }
}

// 수식 맞추기 게임 코드 부분
struct Equation {
    std::string text;
    int answer;
    float x, y;
    float speed;
    bool active;
    Color color;
    int score;

    bool showCorrect = false;
    float correctTimer = 0.0f;
    std::string correctMessage = "Correct!";
};

std::vector<Equation> equations;
std::string inputBuffer;
int score = 0;
int lives = 3;
float spawnTimer = 0.0f;
float spawnInterval = 2.0f;
bool gameOver = false;

bool showDifficultyMessage = false;
float difficultyMessageTime = 0.0f;

Equation GenerateEquation() {
    int a, b, answer;
    char op;
    std::ostringstream oss;

    std::vector<std::string> types = { "easy" };
    if (score >= 150) types.push_back("medium");
    if (score >= 300) types.push_back("hard");

    std::string type = types[GetRandomValue(0, types.size() - 1)];

    Color eqColor = DARKGRAY;
    int eqScore = 10;

    if (type == "easy") {
        a = GetRandomValue(1, 9);
        b = GetRandomValue(1, 9);
        op = (GetRandomValue(0, 1) == 0) ? '+' : '-';
        if (op == '+') {
            answer = a + b;
        }
        else {
            if (a < b) std::swap(a, b);
            answer = a - b;
        }
        eqColor = DARKGRAY;
        eqScore = 10;
    }
    else if (type == "medium") {
        a = GetRandomValue(1, 9);
        b = GetRandomValue(1, 9);
        answer = a * b;
        oss << a << " * " << b;
        eqColor = BLUE;
        eqScore = 20;
    }
    else { // hard
        a = GetRandomValue(10, 99);
        b = GetRandomValue(10, 99);
        op = (GetRandomValue(0, 1) == 0) ? '+' : '-';
        if (op == '+') {
            answer = a + b;
        }
        else {
            if (a < b) std::swap(a, b);
            answer = a - b;
        }
        eqColor = RED;
        eqScore = 30;
    }

    if (type != "medium") {
        oss << a << " " << op << " " << b;
    }

    std::string expr = oss.str();
    int textWidth = MeasureText(expr.c_str(), 30);
    int minX = 100;
    int maxX = 700 - textWidth;
    float x = (float)GetRandomValue(minX, maxX > minX ? maxX : minX);

    return { expr, answer, x, 0.0f, 100.0f, true, eqColor, eqScore };
}

void ResetGame() {
    score = 0;
    lives = 3;
    equations.clear();
    inputBuffer.clear();
    spawnTimer = 0.0f;
    spawnInterval = 2.0f;
    gameOver = false;
    showDifficultyMessage = false;
    difficultyMessageTime = 0.0f;
}

int board[N][N];
int fixed[N][N];
int solution[N][N];
int selectedRow = -1, selectedCol = -1;
int selectedNumber = -1;  // 선택된 숫자 (클릭한 숫자)

int boardOffsetX;
int boardOffsetY;
int numberButtonOffsetY;

int count = 0; // 정답 수
bool showWinMessage = false;
bool showTryAgainMessage = false;
bool isGameFinished = false;  // 게임이 끝났는지 여부

// 타이머 관련 변수
float timer = 0.0f;  // 타이머 (초)
float timerSpeed = 1.0f;  // 타이머 속도 (초/프레임)
int count_solutions(int limit);  // 헤더 선언 추가

// ---------------- 스도쿠 로직 ----------------

int is_valid(int row, int col, int num) {
    for (int x = 0; x < N; x++) {
        if (board[row][x] == num || board[x][col] == num)
            return 0;
    }

    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[startRow + i][startCol + j] == num)
                return 0;

    return 1;
}

int fill_board() {
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (board[row][col] == 0) {
                int nums[N];
                for (int i = 0; i < N; i++) nums[i] = i + 1;

                for (int i = N - 1; i > 0; i--) {
                    int j = rand() % (i + 1);
                    int tmp = nums[i];
                    nums[i] = nums[j];
                    nums[j] = tmp;
                }

                for (int i = 0; i < N; i++) {
                    int num = nums[i];
                    if (is_valid(row, col, num)) {
                        board[row][col] = num;
                        if (fill_board()) return 1;
                        board[row][col] = 0;
                    }
                }
                return 0;
            }
        }
    }
    return 1;
}

void remove_numbers(int count) {
    while (count > 0) {
        int row = rand() % N;
        int col = rand() % N;

        if (board[row][col] == 0) continue;

        int backup = board[row][col];
        board[row][col] = 0;
        fixed[row][col] = 0;

        int solutions = count_solutions(2);
        if (solutions != 1) {
            // 해답이 유일하지 않다면 복원
            board[row][col] = backup;
            fixed[row][col] = 1;
        }
        else {
            count--;
        }
    }
}

int solve_sudoku() {
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (board[row][col] == 0) {
                for (int num = 1; num <= 9; num++) {
                    if (is_valid(row, col, num)) {
                        board[row][col] = num;
                        if (solve_sudoku()) return 1;
                        board[row][col] = 0;
                    }
                }
                return 0;
            }
        }
    }
    return 1;
}

int is_board_full() {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (board[i][j] == 0)
                return 0;
    return 1;
}

int check_solution() {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (board[i][j] != solution[i][j])
                return 0;
    return 1;
}


int count_solutions_util(int row, int col, int* count, int limit) {
    if (*count >= limit) return *count;

    if (row == N) {
        (*count)++;
        return *count;
    }

    if (board[row][col] != 0) {
        int nextCol = (col + 1) % N;
        int nextRow = (col == N - 1) ? row + 1 : row;
        return count_solutions_util(nextRow, nextCol, count, limit);
    }

    for (int num = 1; num <= 9; num++) {
        if (is_valid(row, col, num)) {
            board[row][col] = num;

            int nextCol = (col + 1) % N;
            int nextRow = (col == N - 1) ? row + 1 : row;

            count_solutions_util(nextRow, nextCol, count, limit);
        }
        board[row][col] = 0;
    }
    return *count;
}

int count_solutions(int limit) {
    int tempCount = 0;
    count_solutions_util(0, 0, &tempCount, limit);
    return tempCount;
}

// ---------------- Raylib 렌더링 ----------------

void draw_board() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            Rectangle cell = { boardOffsetX + j * CELL_SIZE, boardOffsetY + i * CELL_SIZE, CELL_SIZE, CELL_SIZE };

            // 숫자 일치 하이라이트
            if (selectedNumber != -1 && board[i][j] == selectedNumber) {
                DrawRectangleRec(cell, Fade(YELLOW, 0.4f));
            }

            // 선택된 셀 강조
            if (i == selectedRow && j == selectedCol)
                DrawRectangleRec(cell, Fade(YELLOW, 0.3f));

            DrawRectangleLinesEx(cell, 1, GRAY);

            // 선택한 셀 하이라이트는 위에 덮기
            if (i == selectedRow && j == selectedCol) {
                DrawRectangleRec(cell, Fade(ORANGE, 0.4f));
            }

            if (board[i][j] != 0) {
                char num[2];
                sprintf(num, "%d", board[i][j]);

                // 텍스트의 크기 측정
                int textWidth = MeasureText(num, 30);
                int textHeight = 30; // 폰트 크기

                // 텍스트의 가운데 위치 계산
                int x = boardOffsetX + j * CELL_SIZE + (CELL_SIZE - textWidth) / 2;
                int y = boardOffsetY + i * CELL_SIZE + (CELL_SIZE - textHeight) / 2;

                if (fixed[i][j]) {
                    // 고정된 숫자는 더 진하게 표시
                    DrawText(num, x - 1, y, 30, BLACK);
                    DrawText(num, x + 1, y, 30, BLACK);
                    DrawText(num, x, y - 1, 30, BLACK);
                    DrawText(num, x, y + 1, 30, BLACK);
                    DrawText(num, x, y, 30, BLACK);
                }
                else {
                    DrawText(num, x, y, 30, DARKGRAY);
                }
            }
        }
    }

    // 3x3 블록 굵은 선
    for (int i = 0; i <= N; i++) {
        int thick = (i % 3 == 0) ? 3 : 1;
        Vector2 start = { boardOffsetX + i * CELL_SIZE, boardOffsetY };
        Vector2 end = { boardOffsetX + i * CELL_SIZE, boardOffsetY + BOARD_SIZE };
        DrawLineEx(start, end, thick, BLACK);
        Vector2 start1 = { boardOffsetX, boardOffsetY + i * CELL_SIZE };
        Vector2 end1 = { boardOffsetX + BOARD_SIZE, boardOffsetY + i * CELL_SIZE };
        DrawLineEx(start1, end1, thick, BLACK);
    }
}

void draw_numbers() {
    for (int i = 0; i < 9; i++) {
        Rectangle btn = { boardOffsetX + i * CELL_SIZE, numberButtonOffsetY, CELL_SIZE, 60 };
        DrawRectangleRec(btn, LIGHTGRAY);
        DrawRectangleLinesEx(btn, 1, DARKGRAY);

        char num[2];
        sprintf(num, "%d", i + 1);

        // 텍스트 크기 측정
        int textWidth = MeasureText(num, 30);
        int textHeight = 30;

        // 가운데 정렬된 위치 계산
        int textX = btn.x + (CELL_SIZE - textWidth) / 2;
        int textY = btn.y + (60 - textHeight) / 2;

        DrawText(num, textX, textY, 30, BLACK);
    }
}

void draw_timer() {
    // 타이머를 보드 위쪽 오른쪽에 표시
    char timeStr[20];
    sprintf(timeStr, "Time: %.1f", timer);
    DrawText(timeStr, SCREEN_WIDTH - 200, 20, 30, BLACK);  // 위치를 화면의 오른쪽 상단으로 설정
}

void draw_win_box() {
    // showWinMessage가 false일 때 메시지 박스를 그리지 않음
    if (!showWinMessage) {
        return;
    }

    int boxWidth = 500;
    int boxHeight = 200;
    int boxX = (SCREEN_WIDTH - boxWidth) / 2;
    int boxY = SCREEN_HEIGHT / 3;

    // 반투명 배경 박스
    DrawRectangle(boxX, boxY, boxWidth, boxHeight, Fade(LIGHTGRAY, 0.7f));

    // 메시지 출력
    const char* winMsg = "You Win!!!";
    int textWidth = MeasureText(winMsg, 80);
    DrawText(winMsg, boxX + (boxWidth - textWidth) / 2, boxY + 60, 80, GREEN);

    // 닫기 버튼 (절대 좌표 기준)
    Rectangle closeBtn = { boxX + boxWidth - 40, boxY + 10, 30, 30 };
    DrawRectangleRec(closeBtn, RED);
    DrawText("X", closeBtn.x + 8, closeBtn.y + 5, 20, WHITE);

    // 마우스 클릭 처리 (절대 좌표 사용)
    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, closeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        showWinMessage = false;  // X 버튼 클릭 시 메시지 박스를 숨김
        isGameFinished = true;   // 게임 종료 표시
    }
}


// 메인 메뉴
void DrawMainMenu() {
    DrawText("Welcome to Game Selector", 250, 100, 30, BLACK);
    DrawText("1. 2048", 300, 200, 20, BLACK);
    DrawText("2. Math Smash", 300, 250, 20, BLACK);
    DrawText("3. Sudoku", 300, 300, 20, BLACK);
    DrawText("Press '1' or '2' or '3' to select a game.", 250, 350, 20, BLACK);
}

int main() {
    InitWindow(800, 600, "Game Selector");
    SetTargetFPS(60);
    srand(time(NULL));

    bool isInMenu = true;
    bool isIn2048 = false;
    bool isInMathSmash = false;
    bool isInSudoku = false;


    while (!WindowShouldClose()) {
        if (isInMenu) {
            if (IsKeyPressed(KEY_ONE)) {
                isInMenu = false;
                isIn2048 = true;
                AddRandomTile();
                AddRandomTile();
            }
            else if (IsKeyPressed(KEY_TWO)) {
                isInMenu = false;
                isInMathSmash = true;
            }
            else if (IsKeyPressed(KEY_THREE)) {
                isInMenu = false;
                isInSudoku = true;
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawMainMenu();
            EndDrawing();
        }

        if (isIn2048) {
            // 2048 게임 실행
            if (IsKeyPressed(KEY_LEFT)) {
                if (Move(0)) AddRandomTile();
            }
            if (IsKeyPressed(KEY_UP)) {
                if (Move(1)) AddRandomTile();
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                if (Move(2)) AddRandomTile();
            }
            if (IsKeyPressed(KEY_DOWN)) {
                if (Move(3)) AddRandomTile();
            }

            BeginDrawing();
            ClearBackground(Color{ 182, 174, 163, 255 });
            DrawBoard();
            EndDrawing();
        }

        if (isInMathSmash) {
            // 수식 맞추기 게임 실행
            float dt = GetFrameTime();

            if (IsKeyPressed(KEY_R) && gameOver) {
                ResetGame();
            }

            if (!gameOver) {
                spawnTimer += dt;

                if (score >= 150 && !showDifficultyMessage) {
                    showDifficultyMessage = true;
                    difficultyMessageTime = 2.0f;
                    spawnInterval = 1.5f;
                    for (auto& eq : equations)
                        eq.speed *= 1.2f;
                }

                if (showDifficultyMessage) {
                    difficultyMessageTime -= dt;
                    if (difficultyMessageTime <= 0.0f)
                        showDifficultyMessage = false;
                }

                if (spawnTimer >= spawnInterval) {
                    equations.push_back(GenerateEquation());
                    spawnTimer = 0.0f;
                }

                // 입력 처리
                int key = GetCharPressed();
                while (key > 0) {
                    if (key >= '0' && key <= '9')
                        inputBuffer += (char)key;
                    key = GetCharPressed();
                }

                if (IsKeyPressed(KEY_BACKSPACE) && !inputBuffer.empty()) {
                    inputBuffer.pop_back();
                }

                if (IsKeyPressed(KEY_ENTER) && !inputBuffer.empty()) {
                    int inputVal = std::stoi(inputBuffer);

                    for (auto& eq : equations) {
                        if (eq.active && eq.answer == inputVal) {
                            eq.active = false;
                            score += eq.score;
                            eq.showCorrect = true;
                            eq.correctTimer = 0.5f;

                            // 응원 메시지 설정
                            if (eq.y < 200)
                                eq.correctMessage = "Great!";
                            else if (eq.y < 400)
                                eq.correctMessage = "Correct!";
                            else
                                eq.correctMessage = "Good!";
                            break;
                        }
                    }

                    inputBuffer.clear();  // 정답/오답 상관없이 입력 초기화
                }

                for (auto& eq : equations) {
                    if (eq.active) {
                        eq.y += eq.speed * dt;
                        if (eq.y > 600) {
                            eq.active = false;
                            lives--;
                            if (lives <= 0) gameOver = true;
                        }
                    }

                    if (eq.showCorrect) {
                        eq.correctTimer -= dt;
                        if (eq.correctTimer <= 0.0f)
                            eq.showCorrect = false;
                    }
                }
            }

            // ------------ 그리기 ------------ 
            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText(("Score: " + std::to_string(score)).c_str(), 10, 10, 20, DARKGRAY);
            DrawText(("Lives: " + std::to_string(lives)).c_str(), 10, 40, 20, MAROON);
            DrawText(("Input: " + inputBuffer).c_str(), 10, 70, 20, BLUE);

            for (auto& eq : equations) {
                if (eq.active) {
                    DrawText(eq.text.c_str(), eq.x, eq.y, 30, eq.color);
                }
                if (eq.showCorrect) {
                    DrawText(eq.correctMessage.c_str(), eq.x, eq.y + 50, 16, DARKGREEN);
                }
            }

            if (gameOver) {
                DrawText("Game Over!", 300, 300, 40, RED);
                DrawText("Press 'R' to Restart", 300, 350, 20, DARKGRAY);
            }

            if (showDifficultyMessage)
                DrawText("Difficulty Increased!", 300, 550, 20, ORANGE);

            EndDrawing();
        }
        if (isInSudoku) {
            boardOffsetY = 50;
            boardOffsetX = (SCREEN_WIDTH - BOARD_SIZE) / 2;
            numberButtonOffsetY = boardOffsetY + BOARD_SIZE + 20;

            srand(time(NULL));
            InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sudoku - Raylib");
            SetTargetFPS(60);

            fill_board();

            // 정답 저장
            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++)
                    solution[i][j] = board[i][j];

            remove_numbers(40);

            // fixed 값 재설정
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    if (board[i][j] != 0) fixed[i][j] = 1;
                    else fixed[i][j] = 0;
                }
            }

            while (!WindowShouldClose()) {
                Vector2 mouse = GetMousePosition();

                // 타이머 업데이트
                if (!isGameFinished) {
                    timer += timerSpeed / 60.0f;  // 초 단위로 업데이트
                }

                // 보드 클릭: showWinMessage가 true여도 클릭을 인식하도록 수정
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isGameFinished) {
                    // 보드 클릭
                    if (mouse.y >= boardOffsetY && mouse.y < boardOffsetY + BOARD_SIZE &&
                        mouse.x >= boardOffsetX && mouse.x < boardOffsetX + BOARD_SIZE) {
                        selectedRow = (mouse.y - boardOffsetY) / CELL_SIZE;
                        selectedCol = (mouse.x - boardOffsetX) / CELL_SIZE;
                        if (board[selectedRow][selectedCol] != 0) {
                            selectedNumber = board[selectedRow][selectedCol];
                        }
                        else {
                            selectedNumber = -1;
                        }
                    }

                    // 숫자 버튼 입력
                    else if (mouse.y >= numberButtonOffsetY && mouse.y <= numberButtonOffsetY + 60 &&
                        mouse.x >= boardOffsetX && mouse.x < boardOffsetX + 9 * CELL_SIZE &&
                        selectedRow != -1 && selectedCol != -1) {
                        if (fixed[selectedRow][selectedCol] == 0) {
                            int numIndex = (mouse.x - boardOffsetX) / CELL_SIZE;
                            board[selectedRow][selectedCol] = numIndex + 1;
                            selectedNumber = numIndex + 1;
                        }
                    }
                }

                // 보드가 가득차고 게임이 끝나지 않을 때
                if (is_board_full() && !isGameFinished) {
                    if (check_solution()) {
                        isGameFinished = true;  // 게임이 끝났음을 표시
                        showWinMessage = true;  // "You Win!!!" 메시지 박스를 표시
                        showTryAgainMessage = false;  // "Try Again..." 메시지를 숨깁니다
                    }
                    else {
                        showTryAgainMessage = true; // 틀린 경우에만 "Try Again..." 메시지를 표시
                    }
                }

                BeginDrawing();
                ClearBackground(RAYWHITE);

                draw_board();
                draw_numbers();
                draw_timer();  // 타이머를 그립니다.

                // "You Win!!!" 메시지 박스
                if (showWinMessage) {
                    draw_win_box();  // Win 메시지 박스 그리기
                }
                // "Try Again..." 메시지 (틀린 경우)
                else if (showTryAgainMessage) {
                    const char* tryMsg = "Try Again...";
                    int tryWidth = MeasureText(tryMsg, 30);
                    DrawText(tryMsg, (SCREEN_WIDTH - tryWidth) / 2, numberButtonOffsetY + 80, 30, RED);
                }

                EndDrawing();
            }
        }
    }

    CloseWindow();
    return 0;
}
