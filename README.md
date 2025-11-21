# Chroma

**Chroma** helps in simulating different types of color blindness occurring in humans.

The main purpose of this is to simulate 3 major [Color Blindness](https://en.wikipedia.org/wiki/Color_blindness) types: _Protanopia_, _Deuteranopia_ and _Tritanopia_ for our different games and to aid the accessibility team in performing various complex testing.

Following are key features:

- Color simulation on a single monitor. This solution works on top of the game and can be maximized as per requirements.
- Works on all games. No dependency on any specific game or engine.
- High performance. Able to simulate live gameplay up to 60 FPS.
- Accurate results.
- Simulation of all types of color blindness.
- Only available solution which captures live gameplay screen and simulates colorblindness.
- Easy screenshots to log errors.
- Easy and configurable UI.
- For more details look into userguide [here](source/Userguide.pdf).
<br/>

**Download the official Chroma logos [here](assets/logos).**

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
