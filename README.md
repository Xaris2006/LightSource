# LightSource

LightSource is a simple chess GUI that chess players can use to save, open and analyse chess games with also the help of a chess game (default Stockfish). With this program you can create your own Chess Opening Book (COB) files that you can use to improve your perfomance in openings.

## Requirements
- [Visual Studio 2022](https://visualstudio.com) (not strictly required, however included setup scripts only support this)

## Getting Started
Once you've cloned, run `scripts/Setup-LightSource.bat` to generate Visual Studio 2022 solution/project files. Once you've opened the solution, you can run the LightSource project to run the chess board app. To run the LightSource-Lobby project you have to build the LightSource project in Dist configuration and copy its .exe file from 'bin\Dist-windows-x86_64\LightSource' directory to 'LightSource-Lobby\LightSourceApp' directory.

### 3rd party libaries
- [implot](https://github.com/epezent/implot)
- [Walnut](https://github.com/StudioCherno/Walnut)
