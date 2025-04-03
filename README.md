## Note on the Current Status

This project was one of the most technically challenging setups I've worked on.

My goal was to build a DICOM viewer with measurement capability using VTK, Qt, and GDCM — compiled manually with CMake and Visual Studio, without relying on pre-packaged dependencies. I managed to compile all components and launch the viewer interface successfully.

However, due to time constraints, I could not fully resolve the final display issue where the loaded DICOM image appears black. This is likely related to the rendering pipeline or missing window-level settings, and I plan to return to it when possible.

For now, I’ve pushed the current state to GitHub to document my process and make it easier to continue or share with others in the future.

I’ve done my best and learned a lot during the process.

With Regards
