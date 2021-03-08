cppcraft
==========


## Installation

Pull repo, compile for your OS using makefile

### Win64:
Install mingw-builds (dual target) to a folder that makes sense, ie. c:\MingW64.
Include c:\mingw64 in PATH enviroment variable (google it).
Open a console in the cppcraft repo folder, execute:
mingw32-make -j8

Should compile right out of the box for windows (And the .exe is static)

### Linux64:
For linux you need to install xorg-dev, libbass for linux x64 and you need to compile GLFW3
BASS: www.un4seen.com    GLFW: http://glfw.org

(I don't have alot of experience with linux!)

The workspaces are for CodeLite IDE, however feel free to use any IDE you want to

## Generating a world

Unfortunately the generator is currently for windows only.
Star the generator, execute '.all' just to test. '.all' is a batch file, as seen in the batch folder.
The generator will output to a specific folder that may not exist already.
Create the world folder (I believe the structure Worlds/test in the Debug/ folder, same as game client)
If the folder doesn't exist, the generator will still work, the game will be started, and nothing will show up except a blue sky.
If the world was successfully created it should load a small area.

Once the smaller area is created, you might want to create a bigger world: .huge

This operation typically takes 20-30 minutes.

## Playing the game
config.ini contains all the configuration variables.
You typically want to adjust screen size, fullscreen, number of worker threads used and lighting ray length.
Number of lighting rays & length affect performance greatly on the CPU side. Screen size affects GPU greatly.
When not debugging or making rapid changes use windowed mode, otherwise fullscreen offers better experience.
