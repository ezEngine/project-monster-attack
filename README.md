# Monster Attack Game Project

Monster Attack is a demo project for how to make a game with EZ engine. It is heavily inspired by the immensely fun tower defense game [Orcs Must Die!](https://store.steampowered.com/app/102600/Orcs_Must_Die/) (go check that out too!).
The demo is meant both to showcase what the engine can do and to show how various game mechanics can be implemented.

If you are interested in EZ, you are encouraged to check out this project, play around with it and see how things are set up.

However, this project is work-in-progress. If you like, you are welcome to contribute to it.

![Monster Attack Screenshot](MonsterAttack-Main.jpg)

## How to Build

This repository is fully self-sufficient, since it pulls in ezEngine as a submodule. This guarantees that it uses the correct version.

1. Clone this repository:\
   `git clone https://github.com/ezEngine/project-monster-attack.git`
1. Change directory:\
   `cd project-monster-attack`
1. Make sure you have all the necessary submodules:\
   `git submodule init`\
   `git submodule update`
1. Generate ezEngine solution for Visual Studio:\
   `cd ezEngine`\
   `GenerateWin64vs2022.bat`
1. Open solution:
   `ezEngine/Workspace/vs2022x64/ezEngine_vs2022x64.sln`
1. Build Dev or Debug build.
1. Run the editor.
1. In the dashboard under *Projects* select **Browse...** and select the file `project-monster-attack/Monster Attack/ezProject`
1. When asked to build the engine plugin, click *Apply*.
1. Open the *Scene* **Level2** (working title)
1. To edit / compile the C++ plugin, choose `Project > C++ Project > Open Solution`
1. To play the game from the editor, press `Ctrl+F5`
1. To play the game in stand-alone mode, press `Ctrl+R` and choose **Export & Run**.

## Game Controls

* *WSAD* + Mouse for first person movement
* *Shift* to run
* *Space* to jump
* *ESC* to quit the game
* *Left click* to shoot or place a trap
* Use the *number keys* to switch between your gun and the different traps.
* *G* to start the next wave

## Useful Resources

EZ has extensive [online documentation](https://ezengine.net/pages/docs/docs-overview.html). There are also [videos on YouTube](https://www.youtube.com/@ezEngine/videos) about various topics. We also have a [Discord server](https://ezengine.net/pages/contact.html#questions-and-discussions) where we've been describing this project in more detail.

## Future Work & Contributions

This is a list of things that could be done to play around with the demo. Feel free to contribute to the project. If you want to check-in assets, make sure they are under a free license (MIT or CC0). So far we have been using assets from [Quaternius](https://quaternius.com/) and [freesound.org](https://freesound.org/).

### Level Design

* Design different levels, with different phases
* Modular Level pieces:
  * Stairs
  * Pits (water / lava)
* More decorative elements

### Sounds

Additional sounds could be added for these things:

* Monster footsteps
* Monster spawn
* Monster reaches goal
* Monster death
* Level ambience / music
* Different level phase music (build, countdown, fight, win/lose)
* Trap placement
* Player wins
* Player loses

### Particle Effects

* Better hit / blood effects
* Torch fire
* Goal magic portal thingy
* Death explode effect
* Impact effects for darts / magic bullets on different surface types
* Ambient smoke / haze
* Monster reaches goal
* Monster spawn / spawn phase begin

### AI

* Infrastructure for AI will be WIP for some time
* Proper steering AI to avoid other monsters
* Colliders to prevent player from walking through

### Monsters

* Proper skeleton / hit box setup
* Ragdolls
* Different monster meshes and animations (needs free assets)

### Traps

* More trap types
* Better trap placement (proper validation where one can be placed)
* Selling traps functionality

### Shaders

* Highlight shader (for selecting traps to sell)
* Different shading during trap placement (preview)

### UI

* Proper level UI with RmlUI
* Show money, current phase (n of m), playtime
* Show available + selected weapon / trap
* Main menu, with scene selection
* Graphics menu, with rendering options, monitor + resolution selection, etc

### Standalone App / Game Loop

* Gamestate should show main menu when starting stand-alone
* Level progression
* Scene selection and unlocking levels
* Restart level when player loses
* Allow player to restart level any time
