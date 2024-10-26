# Build instructions
## MacOS Desktop
```
$ brew install git cmake pkgconfig libusb
$ cmake -DCMAKE_BUILD_TYPE=DEBUG -S . -B build/default
```

## MacOS XCode
```
$ brew install git cmake pkgconfig libusb
$ cmake -G "Xcode" -DCMAKE_SYSTEM_NAME="iOS" -DCMAKE_PLATFORM_NAME="iOS" -S -DSDL_SHARED=OFF . -B build/ios
```
