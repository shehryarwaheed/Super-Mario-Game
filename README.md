# 🍄 Super Mario Game

A Super Mario-inspired game built in **C++** using the **SFML** framework.

> Developed by:
> - L24-3023 Muhammad Shehryar Waheed
> - L24-3007 Husnain Khan
> - L24-3063 Muazam Mehmood

---

## 🎮 Features

- 📝 **Player Name Entry** — Enter your name before the game starts
- 🏆 **Score System** — Track and save your score
- 🎵 **Sound & Music** — Background music and sound effects
- 🗺️ **Multiple Levels** — Progress through different levels

---

## 🛠️ Prerequisites

Before running the game, make sure you have the following installed:

- [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) (2019 or later)
- [SFML 2.x](https://www.sfml-dev.org/download.php) — Simple and Fast Multimedia Library
- Windows OS (x64)

---

## 🚀 How to Run Locally

### Step 1 — Clone the Repository

> ⚠️ This repo is **read-only**. You can only **fetch/clone** the code, not push changes.

```bash
git clone https://github.com/shehryarwaheed/Super-Mario-Game.git
```

Or download the ZIP directly from the green **Code** button above.

---

### Step 2 — Set Up SFML in Visual Studio

1. Download **SFML 2.x** from [sfml-dev.org](https://www.sfml-dev.org/download.php)
2. Extract it to a folder (e.g. `C:\SFML`)
3. Open Visual Studio → go to your project properties:
   - **C/C++ → General → Additional Include Directories** → Add `C:\SFML\include`
   - **Linker → General → Additional Library Directories** → Add `C:\SFML\lib`
   - **Linker → Input → Additional Dependencies** → Add:
     ```
     sfml-graphics-d.lib
     sfml-window-d.lib
     sfml-system-d.lib
     sfml-audio-d.lib
     ```
4. Copy all `.dll` files from `C:\SFML\bin` into your project's `x64\Debug\` folder

---

### Step 3 — Open & Run the Project

1. Open the project folder
2. Double-click **`game.sln`** to open it in Visual Studio
3. Make sure the configuration is set to **Debug | x64**
4. Press **F5** or click the green ▶ **Start** button

The game window will launch and ask for the player's name. Enjoy! 🎮

---

## 📁 Project Structure

```
Super-Mario-Game/
├── game/
│   ├── main.cpp          # Main game source file
│   ├── font/             # Game fonts
│   ├── image/            # Sprites and images
│   ├── sound/            # Audio files
│   └── score.txt         # Score storage
├── game.sln              # Visual Studio solution file
└── game.vcxproj          # Visual Studio project file
```

---

## 🔒 Contributing & Permissions

This repository is **view/clone only**.

- ✅ You **can** clone or download the code
- ✅ You **can** run it locally
- ❌ You **cannot** push changes to this repository
- ❌ Pull requests are **not accepted**

If you'd like to suggest something, open an **Issue** — but changes to the codebase are managed solely by the original authors.

---

## 📄 License

This project is for **educational purposes only**. All rights reserved by the original authors.
