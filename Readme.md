

# Skibiddi Shooter

Skibiddi Shooter is a fast-paced 2D top-down shooter game built with **C++** and **Raylib**. Players control a character, dodge enemies, and shoot them down to survive waves of increasingly challenging enemies.

## Play the game here: [Skibiddi Shooter on Itch.io](https://marshaldevs.itch.io/skibiddi-shooter)

---

## Features

- Real-time player movement and shooting
- Multiple waves of enemies with increasing difficulty
- Smooth animations with character facing direction based on mouse position
- Player and enemy collision detection
- Sound effects and background music for immersive gameplay
- Enemy spawn logic ensures fair distances from the player

---

## Gameplay

- Move the player using **W/A/S/D** or **Arrow Keys**
- Aim with the **mouse** and shoot with the **left mouse button**
- Survive waves of enemies and try to achieve the highest wave possible
- If the player collides with an enemy, the game resets to the menu

---


### Prerequisites

- **C++17** compiler (GCC/Clang/MSVC)
- **Raylib** library installed ([installation guide](https://www.raylib.com/))
- Audio support for `.mp3` files (used for sound effects and music)



## Controls

| Action     | Keys              |
| ---------- | ----------------- |
| Move Up    | W / Up Arrow      |
| Move Down  | S / Down Arrow    |
| Move Left  | A / Left Arrow    |
| Move Right | D / Right Arrow   |
| Shoot      | Left Mouse Button |
| Aim        | Mouse Movement    |

---

## Project Structure


* `Guy` class handles the player logic and shooting
* `Enemy` class handles enemy movement, drawing, and collision
* Game loop handles state management: **Menu, Gameplay, Exit**
* Sound management via Raylib audio system

---

## Dependencies

* [Raylib](https://www.raylib.com/) - graphics, input, audio
* C++ STL (vector, cmath)

---

## Future Improvements

* Improve modularity and refactor code for better readability
* Add different enemy types with unique behaviors
* Implement power-ups and collectibles
* Add high-score tracking and save/load functionality
* Improve particle effects and visual polish

---

```
