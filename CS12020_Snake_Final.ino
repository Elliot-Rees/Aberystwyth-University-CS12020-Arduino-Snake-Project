#include <AberLED.h>
#include <EEPROM.h>

// --- Game constants ---
#define GRID_SIZE 8
#define TICK_TIME 250  // milliseconds per tick

// --- Direction constants --- 
#define NORTH 0 
#define EAST 1
#define SOUTH 2
#define WEST 3

// --- State constants ---
#define START 0
#define PLAYING 1
#define PAUSED 2
#define LIFELOST 3
#define END 4
#define HIGHSCORE 5

// --- Game settings --- 
#define INITIAL_LIVES 3
#define POINTS_PER_FOOD 10
#define LIFELOST_DISPLAY_TIME 2000  // milliseconds
#define EEPROM_HIGHSCORE_ADDR 0

// --- Model Global variables ---
int headX;        // X position of snake head
int headY;        // Y position of snake head
int direction;    // Current direction (NORTH, EAST, SOUTH, WEST)
unsigned long lastMoveTime;  // Timer for movement
int state;        // Current game state
unsigned long stateTimer;    // Timer for timed states

// --- Obstacles ---
#define NUM_OBSTACLES 3
int obstacleX[NUM_OBSTACLES];  // X positions of obstacles
int obstacleY[NUM_OBSTACLES];  // Y positions of obstacles

// --- Food ---
int foodX;  // X position of food
int foodY;  // Y position of food

// --- Snake body ---
#define MAX_SNAKE_LENGTH 16
int snakeX[MAX_SNAKE_LENGTH];  // X positions of snake segments (index 0 is head)
int snakeY[MAX_SNAKE_LENGTH];  // Y positions of snake segments (index 0 is head)
int snakeLength;               // Current length of snake

// --- Game stats --- 
int lives;       // Current number of lives
int score;       // Current score
int highScore;   // High score (loaded from EEPROM)

// --- Setup ---
void setup() {
  AberLED.begin();
  randomSeed(analogRead(0));
  loadHighScore();
  initGame();
}

// --- Main Loop ---
void loop() {
  handleInput();
  updateModel();
  render();
  AberLED.swap();   // Swap buffers to display the frame
  delay(10); // Helps to slow the game down not to overwhelm processor 
}

// --- Initialise  game ---
void initGame() {
  state = START;
  lives = INITIAL_LIVES;
  score = 0;
}
//  --- Reset snake ---
void resetSnake() {
  // Place head at random location, avoiding edges
  headX = random(1, GRID_SIZE - 1);
  headY = random(1, GRID_SIZE - 1);
  
  // Initialise snake body
  snakeLength = 1;
  snakeX[0] = headX;
  snakeY[0] = headY;
  
  // Set random initial direction
  direction = random(0, 4);
  
  // Initialise  timer
  lastMoveTime = millis();
  
  // Spawn obstacles 
  createObstacles();
  
  // Spawn Food
  createFood();
}

// --- Input Handler --- 
void handleInput() {
  switch (state) {
    case START:
      // Wait for FIRE to begin playing
      if (AberLED.getButtonDown(FIRE)) {
        state = PLAYING;
        resetSnake();
      }
      break;
      
    case PLAYING:
      if (AberLED.getButtonDown(FIRE)) {
        state = PAUSED;
      }
      else if (AberLED.getButtonDown(UP)) {
        direction = NORTH;
      }
      else if (AberLED.getButtonDown(DOWN)) {
        direction = SOUTH;
      }
      else if (AberLED.getButtonDown(LEFT)) {
        direction = WEST;
      }
      else if (AberLED.getButtonDown(RIGHT)) {
        direction = EAST;
      }
      break;
      
    case PAUSED:
      if (AberLED.getButtonDown(FIRE)) {
        state = PLAYING;
        lastMoveTime = millis();  // Reset timer to prevent instant move
      }
      break;
      
    case LIFELOST:
      // No input during LIFELOST  (automatically transitions)
      break;
      
    case END:
      if (AberLED.getButtonDown(FIRE)) {
        state = HIGHSCORE;
      }
      break;
      
    case HIGHSCORE:
      if (AberLED.getButtonDown(FIRE)) {
        initGame();
      }
      break;
  }
}

// --- Update game model ---
void updateModel() {
  switch (state) {
    case START:
      // Nothing to update in START state
      break;
      
    case PLAYING:
      // Check if it's time to move (tick)
      {
        unsigned long currentTime = millis();
        if (currentTime - lastMoveTime >= TICK_TIME) {
          // Update timer
          lastMoveTime = currentTime;
          
          // Move the snake in its current direction
          moveSnake();
          
          // Check for collision with obstacles
          if (checkObstacleCollision()) {
            loseLife();
          }
          
          // Check for self-collision
          if (checkSelfCollision()) {
            loseLife();
          }
          
          // Check if food was eaten
          if (checkFoodEaten()) {
            // Add to score
            score += POINTS_PER_FOOD;
            
            // Grow the snake (up to maximum length)
            if (snakeLength < MAX_SNAKE_LENGTH) {
              snakeLength++;
            }
            createFood();  // Generate new food
          }
          
          // Check for edges and corners
          handleEdges();
        }
      }
      break;
      
    case PAUSED:
      // Nothing to update while paused
      break;
      
    case LIFELOST:
      // Check if display time has elapsed
      {
        unsigned long currentTime = millis();
        if (currentTime - stateTimer >= LIFELOST_DISPLAY_TIME) {
          if (lives > 0) {
            state = PLAYING;
            resetSnake();
          } else {
            state = END;
            updateHighScore();
          }
        }
      }
      break;
      
    case END:
      // Nothing to update in END state
      break;
      
    case HIGHSCORE:
      // Nothing to update in HIGHSCORE state
      break;
  }
}

// --- Lose a life ---
void loseLife() {
  lives--;
  state = LIFELOST;
  stateTimer = millis();
}

// --- Move snake --- 
void moveSnake() {
  // Calculate new head position
  int newHeadX = headX;
  int newHeadY = headY;
  
  switch (direction) {
    case NORTH:
      newHeadY--;
      break;
    case EAST:
      newHeadX++;
      break;
    case SOUTH:
      newHeadY++;
      break;
    case WEST:
      newHeadX--;
      break;
  }
  
  // Shift all body segments back by one position
  // Start from the tail and move towards the head
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }
  
  // Place new head position at index 0
  snakeX[0] = newHeadX;
  snakeY[0] = newHeadY;
  headX = newHeadX;
  headY = newHeadY;
}

// --- Edge and corner collision handler --- 
void handleEdges() {
  if (headX == 0 && headY == 0) {
    // Top-left corner
    if (direction == NORTH) direction = EAST;
    else if (direction == WEST) direction = SOUTH;
  }
  else if (headX == GRID_SIZE - 1 && headY == 0) {
    // Top-right corner
    if (direction == NORTH) direction = WEST;
    else if (direction == EAST) direction = SOUTH;
  }
  else if (headX == GRID_SIZE - 1 && headY == GRID_SIZE - 1) {
    // Bottom-right corner
    if (direction == SOUTH) direction = WEST;
    else if (direction == EAST) direction = NORTH;
  }
  else if (headX == 0 && headY == GRID_SIZE - 1) {
    // Bottom-left corner
    if (direction == SOUTH) direction = EAST;
    else if (direction == WEST) direction = NORTH;
  }
  // Check if at an edge (turn randomly left or right)
  else if (headX == 0 && direction == WEST) {
    turnRandomly();
  }
  else if (headX == GRID_SIZE - 1 && direction == EAST) {
    turnRandomly();
  }
  else if (headY == 0 && direction == NORTH) {
    turnRandomly();
  }
  else if (headY == GRID_SIZE - 1 && direction == SOUTH) {
    turnRandomly();
  }
}

// Turn randomly left or right
void turnRandomly() {
  if (random(0, 2) == 0) {
    // Turn left
    direction = (direction + 3) % 4;
  } else {
    // Turn right
    direction = (direction + 1) % 4;
  }
}

// --- Spawn Obstacles --- 
void createObstacles() {
  for (int i = 0; i < NUM_OBSTACLES; i++) {
    bool validPosition = false;
    
    while (!validPosition) {
      // Generate random position
      obstacleX[i] = random(0, GRID_SIZE);
      obstacleY[i] = random(0, GRID_SIZE);
      
      // Check if position is valid:
      // 1. Not within 3 squares of snake head
      int distX = abs(obstacleX[i] - headX);
      int distY = abs(obstacleY[i] - headY);
      bool farFromHead = (distX > 3 || distY > 3);
      
      // 2. Not on the same position as another obstacle
      bool notOnOtherObstacle = true;
      for (int j = 0; j < i; j++) {
        if (obstacleX[i] == obstacleX[j] && obstacleY[i] == obstacleY[j]) {
          notOnOtherObstacle = false;
          break;
        }
      } 
      // Position is valid if both rules are met
      validPosition = farFromHead && notOnOtherObstacle;
    }
  }
}

// --- Obstacle Collision handler --- 
bool checkObstacleCollision() {
  for (int i = 0; i < NUM_OBSTACLES; i++) {
    if (headX == obstacleX[i] && headY == obstacleY[i]) {
      return true;  // Collision detected
    }
  }
  return false;  // No collision
}

// --- Food spawner ---
void createFood() {
  bool validPosition = false;
  
  while (!validPosition) {
    // Generate random position
    foodX = random(0, GRID_SIZE);
    foodY = random(0, GRID_SIZE);
    
    // Check if position is valid:
    // 1. Not on any part of the snake
    bool notOnSnake = true;
    for (int i = 0; i < snakeLength; i++) {
      if (foodX == snakeX[i] && foodY == snakeY[i]) {
        notOnSnake = false;
        break;
      }
    }
    
    // 2. Not on any obstacle
    bool notOnObstacle = true;
    for (int i = 0; i < NUM_OBSTACLES; i++) {
      if (foodX == obstacleX[i] && foodY == obstacleY[i]) {
        notOnObstacle = false;
        break;
      }
    }
    
    // Position is valid if both rules are met
    validPosition = notOnSnake && notOnObstacle;
  }
}

// --- Check if snake ate --- 
bool checkFoodEaten() {
  return (headX == foodX && headY == foodY);
}

// --- Self collision check --- 
bool checkSelfCollision() {
  // Start from index 1 (skip the head at index 0)
  for (int i = 1; i < snakeLength; i++) {
    if (headX == snakeX[i] && headY == snakeY[i]) {
      return true;  // Collision with own body
    }
  }
  return false;  // No collision
}

// -- Read EEPROM --- 
void loadHighScore() {
  // Read two bytes for the high score (int is 2 bytes)
  highScore = EEPROM.read(EEPROM_HIGHSCORE_ADDR) | 
              (EEPROM.read(EEPROM_HIGHSCORE_ADDR + 1) << 8);
  
  // Validate high score (in case EEPROM is uninitialized)
  if (highScore < 0 || highScore > 10000) {
    highScore = 0;
  }
}

// --- EEPROM writer ---
void updateHighScore() {
  if (score > highScore) {
    highScore = score;
    // Save to EEPROM
    EEPROM.write(EEPROM_HIGHSCORE_ADDR, highScore & 0xFF);
    EEPROM.write(EEPROM_HIGHSCORE_ADDR + 1, (highScore >> 8) & 0xFF);
  }
}

// --- Draw Number ---
void drawNumber(int num, int startX, int startY) {
  int displayNum = min(num, GRID_SIZE * GRID_SIZE);  // Cap at display size
  
  for (int i = 0; i < displayNum && i < 8; i++) {
    int x = startX + (i % 4);
    int y = startY + (i / 4);
    if (x < GRID_SIZE && y < GRID_SIZE) {
      AberLED.set(x, y, GREEN);
    }
  }
}

// --- Render handler --- 
void render() {
  AberLED.clear();
  AberLED.clearText();
  
  switch (state) {
    case START:
      for (int i = 0; i < GRID_SIZE; i++) {
        AberLED.set(i, 0, GREEN);              // Top edge
        AberLED.set(i, GRID_SIZE - 1, GREEN);  // Bottom edge
        AberLED.set(0, i, GREEN);              // Left edge
        AberLED.set(GRID_SIZE - 1, i, GREEN);  // Right edge
      }
      AberLED.addToText("SNAKE GAME");
      AberLED.addToText("\nPress FIRE");
      break;
      
    case PLAYING:
    case PAUSED:
      // Draw all snake segments
      for (int i = 0; i < snakeLength; i++) {
        AberLED.set(snakeX[i], snakeY[i], GREEN);
      }
      
      // Draw obstacles
      for (int i = 0; i < NUM_OBSTACLES; i++) {
        AberLED.set(obstacleX[i], obstacleY[i], RED);
      }
      
      // Draw food
      AberLED.set(foodX, foodY, YELLOW);
      
      // Display score and lives
      AberLED.addToText("Score: ");
      AberLED.addToText(score);
      AberLED.addToText("\nLives: ");
      AberLED.addToText(lives);
      AberLED.addToText("\nLength: ");
      AberLED.addToText(snakeLength);
      
      if (state == PAUSED) {
        AberLED.addToText("\nPAUSED");
      }
      break;
      
    case LIFELOST:
      // Display remaining lives
      for (int i = 0; i < lives && i < GRID_SIZE; i++) {
        AberLED.set(i, GRID_SIZE / 2, RED);
      }
      AberLED.addToText("Life Lost!");
      AberLED.addToText("\nLives: ");
      AberLED.addToText(lives);
      break;
      
    case END:
      // Display END screen
      for (int i = 0; i < GRID_SIZE; i++) {
        AberLED.set(i, 0, RED);              // Top edge
        AberLED.set(i, GRID_SIZE - 1, RED);  // Bottom edge
        AberLED.set(0, i, RED);              // Left edge
        AberLED.set(GRID_SIZE - 1, i, RED);  // Right edge
      }
      AberLED.addToText("GAME OVER");
      AberLED.addToText("\nScore: ");
      AberLED.addToText(score);
      AberLED.addToText("\nPress FIRE");
      break;
      
    case HIGHSCORE:
      // Display high score
      for (int i = 0; i < GRID_SIZE; i++) {
        AberLED.set(i, 0, YELLOW);              // Top edge
        AberLED.set(i, GRID_SIZE - 1, YELLOW);  // Bottom edge
        AberLED.set(0, i, YELLOW);              // Left edge
        AberLED.set(GRID_SIZE - 1, i, YELLOW);  // Right edge
      }
      AberLED.addToText("HIGH SCORE");
      AberLED.addToText("\n");
      AberLED.addToText(highScore);
      AberLED.addToText("\n\nYour Score: ");
      AberLED.addToText(score);
      if (score == highScore && score > 0) {
        AberLED.addToText("\nNEW RECORD!");
      }
      break;
  }
}