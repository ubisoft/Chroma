# Chroma

**Chroma** helps in simulating different types of color blindness occurring in society.

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


# Issues & Resolutions

1. error C2039: 'wait_for': is not a member of 'winrt::impl'
- This issue may happen because our CPPWinRT librrary might be a bit old. 
- To solve this try to install nuget package - Microsoft.windows.cppwinrt package.
