# CS12020 Snake Game - Arduino Edition

A classic Snake game implemented on Arduino using the AberLED 8x8 TFT display. Navigate the snake, collect food, avoid obstacles, and compete for the high score!

## Overview

This project recreates the Snake game on an Arduino microcontroller with a visual interface on an 8x8 TFT matrix. The game features obstacles, a lives system, score tracking, and persistent high score storage using EEPROM.

## Features

- **Classic Snake Gameplay**: Navigate your snake around the grid to collect food and grow longer
- **Obstacle Avoidance**: Navigate around 3 randomly placed obstacles per round
- **Lives System**: Start with 3 lives; lose one when hitting obstacles or yourself
- **Score Tracking**: Earn 10 points for each food item collected
- **High Score Storage**: High scores are saved to Arduino's EEPROM memory
- **State Machine**: Multiple game states (START, PLAYING, PAUSED, LIFELOST, END, HIGHSCORE)
- **Edge Wrapping**: Snake automatically turns at edges rather than dying
- **Pause Feature**: Press FIRE to pause and resume gameplay

## Hardware Requirements

- Arduino Microcontroller (with AberLED library support)
- AberLED 8x8 TFT Matrix Display
- Push buttons for game input:
  - FIRE (start/pause)
  - UP, DOWN, LEFT, RIGHT (directional controls)

## Software Requirements

- Arduino IDE
- AberLED Library (included in project dependencies)
- EEPROM Library (included with Arduino)

## Installation

1. Clone or download this repository
2. Open `CS12020_Snake_Final.ino` in the Arduino IDE
3. Install the AberLED library:
   - Sketch → Include Library → Manage Libraries
   - Search for "AberLED" and install
4. Connect your Arduino board and AberLED display
5. Select your board type: Tools → Board
6. Select the appropriate COM port: Tools → Port
7. Click Upload to flash the code to your Arduino

## How to Play

### Starting the Game
1. Power on the Arduino
2. The display shows "SNAKE GAME - Press FIRE"
3. Press the FIRE button to begin

### Gameplay
- Use **UP**, **DOWN**, **LEFT**, **RIGHT** buttons to control the snake's direction
- Collect **YELLOW** food items to earn points and grow longer
- Avoid **RED** obstacles and your own body
- Press **FIRE** at any time to pause/resume the game

### Game Over
- You lose one life when hitting an obstacle or yourself
- After losing all 3 lives, the game ends
- Press FIRE to view your high score
- Press FIRE again to return to the start screen

## Game Mechanics

### Scoring
- **Food Collection**: +10 points per food item
- **Maximum Score**: Limited by maximum snake length (16 segments)

### Snake Behavior
- Snake starts at a random position with length 1
- Direction is randomized on each level restart
- Snake grows by 1 segment for each food item collected (up to 16 maximum)
- At grid edges, the snake automatically turns left or right randomly

### Obstacles
- 3 obstacles spawn randomly at the start of each round
- Obstacles never spawn within 3 squares of the snake's head
- Obstacles never overlap with each other
- Colliding with any obstacle costs a life

### Edge Handling
- When the snake hits an edge, it automatically turns
- At corners, specific turn logic applies:
  - Top-left: NORTH→EAST, WEST→SOUTH
  - Top-right: NORTH→WEST, EAST→SOUTH
  - Bottom-right: SOUTH→WEST, EAST→NORTH
  - Bottom-left: SOUTH→EAST, WEST→NORTH

## Game States

| State | Description | Next State |
|-------|-------------|-----------|
| **START** | Initial screen with instructions | PLAYING (on FIRE) |
| **PLAYING** | Active gameplay | PAUSED, LIFELOST |
| **PAUSED** | Game suspended | PLAYING (on FIRE) |
| **LIFELOST** | Life lost screen (2 seconds) | PLAYING or END |
| **END** | Game over screen | HIGHSCORE (on FIRE) |
| **HIGHSCORE** | High score display | START (on FIRE) |

## Display Colors

- **GREEN**: Snake body segments
- **YELLOW**: Food items
- **RED**: Obstacles and edge indicators

### Game Loop Timing
- **Main Loop**: 10ms delay per frame
- **Game Tick**: 250ms interval for snake movement
- **Life Lost Duration**: 2 seconds display time

### Memory Usage
- Maximum snake length: 16 segments
- Grid size: 8x8 pixels
- High score storage: 2 bytes in EEPROM (address 0-1)



### Game State
- `lives`: Remaining lives
- `score`: Current game score
- `highScore`: Best score (loaded from EEPROM)
- `state`: Current game state

### Obstacles & Food
- `obstacleX[]`, `obstacleY[]`: Obstacle positions
- `foodX`, `foodY`: Food position

## Troubleshooting

**Game won't start?**
- Ensure AberLED library is properly installed
- Check that the FIRE button is connected correctly
- Verify EEPROM isn't corrupted; try uploading with a blank EEPROM

**Snake not responding to controls?**
- Check that directional buttons are properly wired
- Ensure buttons are connected to the correct Arduino pins as defined in AberLED library

**Display issues?**
- Verify AberLED display is properly connected
- Check LED matrix orientation and polarity
- Try resetting the Arduino

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details. 