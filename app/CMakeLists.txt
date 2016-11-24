# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/cpp/native-lib.cpp

             src/main/cpp/MediatedSM2.cpp
             src/main/cpp/MediatedSM2Client.cpp
             src/main/cpp/MediatedSM2Service.cpp
             src/main/cpp/MediatedSM2Stub.cpp


             #src/main/cpp/AppValidate.cpp
             #src/main/cpp/ExtractAndParseFile.cpp
             #src/main/cpp/unzip.c
             #src/main/cpp/Base64.cpp
             #src/main/cpp/ioapi.cpp

             src/main/cpp/DeviceInfo.cpp
             src/main/cpp/DeviceInfo1.cpp
             src/main/cpp/3rd/qr/bitstream.c
             src/main/cpp/3rd/qr/mask.c
             src/main/cpp/3rd/qr/mmask.c
             src/main/cpp/3rd/qr/mqrspec.c
             src/main/cpp/3rd/qr/qrencode.c
             src/main/cpp/3rd/qr/qrinput.c
             src/main/cpp/3rd/qr/qrspec.c
             src/main/cpp/3rd/qr/rscode.c
             src/main/cpp/3rd/qr/split.c
             src/main/cpp/3rd/EllipticCurve.cpp
             src/main/cpp/3rd/jpge.cpp
             src/main/cpp/3rd/Mpi.cpp
             src/main/cpp/3rd/sm3hash.cpp
             src/main/cpp/3rd/SMS4.cpp
             )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
