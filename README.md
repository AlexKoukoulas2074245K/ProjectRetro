# ProjectRetro - A 3D Reinterpretation of Pokemon Yellow

![Pallet image](res/screenshots/pallet_screenshot.png)

## The Story
Having always had a blast replaying and remembering the joy that was to play Pokemon Yellow, I decided to give my own spin to the original game, by 3D-fying many elements of the Overworld maps, while emulating others as they appeared in the original game. 

The project originally started as an ECS tech test, with the intention being to discover how applicable a pure ECS framework would be in such a scenario (cloning an old Gameboy Color Game). This is the reason behind most of the ECS-oriented code sprinked across the repo. 

## Implementation
The project is being developed using C++ & SDL2 and is Windows/MacOS compatible. 

## Running the project locally

### Windows

#### Dependencies
* Make sure you have CMake with version >= 3.1
* Make sure you have Visual studio 2015 installed. If a newer version is installed you will have to specify the respective generator inside make_project.bat.

#### Instructions
1) Clone the project
2) Navigate to the root of the project
3) Run the bat file make_project.bat
4) The generated project will reside inside project_files
5) Open the sln file with visual studio
6) Compile and run


### MacOS

#### Dependencies
* Make sure you have CMake with version >= 3.1
* The CMake will look for the SDL frameworks under /Users/{username}/Library/Frameworks: SDL2_image, SDL2_ttf, SDL2, SDL2_mixer

#### Instructions
1) Clone the project
2) Navigate to the root of the project
3) Run make_project.sh
4) The generated project will reside inside project_files
5) Open the xcodeproj file with XCode
6) Compile and run

## Progress
At the time of writing, most of the basic flows that take place in the Overworld (i.e. not Battle) are in place, with the exception of most scripted events (e.g. Pikachu catching sequence). Pallet town, including all houses are mostly complete. Route 1 is complete in terms of models and geometry, however it is missing other NPCs.

Most of the work currently done revolves around the precise emulation of battle mechanics. This is the most tedious and time-consuming aspect of the project, from making sure Gen 1 specific battle bugs are present, to animation timing, and extraction of move animations.

![Battle image](res/screenshots/battle_screenshot.png)

### Characters
* Character movement and interaction works as expected.

### Locations (including houses and indoor areas)
* Pallet town
* Route 1
* Viridian City
* Route 22 (except room leading to victory road)
* Route 2 (viridian side)

### Encounters
* Combat is generally well-emulated at this point, except for burn, and freeze statuses
* The moves are added to the game incrementally, the more areas and wild encounters are added. All moves with animation folders under res/textures/battle_animations should be working properly in battle.

### Misc
* Flows and scripted events are also added incrementally as more levels are built. The major common flows that are currently missing at this stage of content creation are the Poke-mart buying/selling flows, as well as the PC (pokemon deposit/withdrawals, etc..) flow.
