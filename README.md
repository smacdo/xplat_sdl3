# Build instructions
## MacOS Desktop
```
$ brew install git cmake pkgconfig libusb
$ cmake -DCMAKE_BUILD_TYPE=DEBUG -S . -B build/generic
```

## MacOS XCode
```
$ brew install git cmake pkgconfig libusb
$ cmake -G "Xcode" -DCMAKE_SYSTEM_NAME="iOS" -S . -B build/ios
$ open build/ios/xplat_sdl3.xcodeproj
```
