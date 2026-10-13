SlintForge Gallery - Visual C++ Runtime

Gallery setup only installs SlintForge files. It does not install, repair,
or update Microsoft Visual C++ Runtime.

If Gallery reports that the runtime is missing or outdated:
1. Open the SlintForge installation folder.
2. Open the prerequisites folder.
3. Manually run vc_redist.x64.exe and complete Microsoft's installation.
4. Start Gallery-Rust.exe or Gallery-Cpp.exe again.

The default installation folder is:
%LOCALAPPDATA%\Programs\SlintForge

Use the "SlintForge Runtime Instructions" Start menu item to find this file.
An existing compatible runtime does not need to be reinstalled.
Visual Studio, MSVC Build Tools, Rust, and CMake are not needed to run Gallery.

Windows may take several minutes to verify the signed Microsoft packages,
especially in an offline Sandbox. This wait occurs only when you manually
install the runtime and is separate from SlintForge setup.
