This is a simple SDL3 game with cross platform support using SDL3, and can be used as the starting
position for other SDL3 games.

# Build instructions
## Mac/Linux Desktop
Run the following commands in a terminal. Replace `brew` with your system's 
package manager if you're running on Liunux.

```
$ brew install git cmake pkgconfig libusb
$ cmake -DCMAKE_BUILD_TYPE=DEBUG -S . -B build/generic
$ cd build/generic
$ make && DEBUG/xplat_sdl3.app/Contents/MacOS/xplat_sdl3
```

## iOS
Run the following commands in a terminal:
```
$ brew install git cmake pkgconfig libusb
$ cmake -G "Xcode" -DCMAKE_SYSTEM_NAME="iOS" -S . -B build/ios
$ open build/ios/xplat_sdl3.xcodeproj
```

After opening XCode up you will need to configure app signing. Click on `xplat_sdl3` on the left
panel, and then select `xplat_sdl` under targets in the middle panel. Then click the 
`Signing & Capabilities` tab to the right, check `Automatically manage signing` and finally select
your team from the `Team` dropdown list.

Click `run` and you should be good to go!

# Feedback
Please report any issues, bugs or improvements via the issue tracker. 