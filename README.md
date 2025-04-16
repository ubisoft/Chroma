# Chroma

**Chroma** (developed by Ubisoft) helps in simulating different types of color blindness occurring in society.
![03 Hero Asset - Secondary 1](https://github.com/user-attachments/assets/24da16ce-ee0d-42d2-a6c4-d3329086e095)

Main purpose of this is to simulate 3 major [Color Blindness](https://en.wikipedia.org/wiki/Color_blindness) types _Protanopia_, _Deuteranopia_ and _Tritanopia_ for our different games and aid accessibility team in performing various complex testing.

Following are key features:

- Color Simulation on single monitor. This solution works on top of game and can be maximized as per requirement.
- Work on all games. No dependency on any specific game or engine.
- High performance. Able to simulation live gameplay upto 60 FPS.
- Accurate results. 
- Simulation of all type of color blind forms.
- Only available solution which capture screen live gameplay screen and simulate.
- Easy screenshot to log error.
- Easy and configurable UI.




- For more details look into userguide [here](source/Userguide.pdf).

  Download the official Chroma logos [here](assets/logos).

## Known Issue During CMake Process
If you encounter the following error while running CMake without Visual Studio 2022:

```
error C2039: 'wait_for': is not a member of 'winrt::impl'
```

This issue may occur due to an outdated `CPPWinRT` library. To resolve it, install the `Microsoft.Windows.CppWinRT` NuGet package using the following command:

```sh
nuget install Microsoft.Windows.CppWinRT
```

Alternatively, ensure that your development environment is using an updated version of `CPPWinRT`. **The best option to avoid this issue is to use Visual Studio 2022.**
