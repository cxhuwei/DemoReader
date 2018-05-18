# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
include_directories(
        src/main/cpp/Common/cximage
        src/main/cpp/Common/cximage/jasper
        src/main/cpp/Common/cximage/jpeg
        src/main/cpp/Common/cximage/zlib
        src/main/cpp/Common/cximage/CxImage
        src/main/cpp/Common/freetype/include
        src/main/cpp/Common/bzip2-1.0.6
        src/main/cpp/Common/libiconv-1.14
        src/main/cpp/Common/libiconv-1.14/include
        src/main/cpp/Common/libiconv-1.14/lib
        src/main/cpp/Common/libiconv-1.14/libcharset/include
        src/main/cpp/Common/zlib-1.2.5/contrib/minizip
        src/main/cpp/Common/tinyxml
        src/main/cpp/SZDev
        src/main/cpp/SZDev/myfDev
)

aux_source_directory (src/main/cpp/ SRC)
aux_source_directory (src/main/cpp/Common/cximage/CxImage OTHER_SRC)
aux_source_directory (src/main/cpp/Common/cximage/j2k OTHER_SRC)
aux_source_directory (src/main/cpp/Common/cximage/jasper OTHER_SRC)
aux_source_directory (src/main/cpp/Common/cximage/jbig OTHER_SRC)
aux_source_directory (src/main/cpp/Common/cximage/jpeg OTHER_SRC)
aux_source_directory (src/main/cpp/Common/cximage/png OTHER_SRC)
aux_source_directory (src/main/cpp/Common/cximage/zlib OTHER_SRC)

aux_source_directory (src/main/cpp/Common/bzip2-1.0.6 OTHER_SRC)


aux_source_directory (src/main/cpp/Common/tinyxml OTHER_SRC)

#aux_source_directory (src/main/cpp/Common/zlib-1.2.5 OTHER_SRC)
aux_source_directory (src/main/cpp/Common/zlib-1.2.5/contrib/minizip OTHER_SRC)

aux_source_directory (src/main/cpp/SZDev OTHER_SRC)

list(APPEND SRC ${OTHER_SRC})
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftsystem.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftinit.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftdebug.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftbase.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftbbox.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftglyph.c )
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftbitmap.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftcid.c )
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftfstype.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftgasp.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftgxval.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftlcdfil.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftmm.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftotval.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftpatent.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftstroke.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftsynth.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/fttype1.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/base/ftxf86.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/cff/cff.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/cid/type1cid.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/sfnt/sfnt.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/truetype/truetype.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/type1/type1.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/type42/type42.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/truetype/truetype.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/raster/raster.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/smooth/smooth.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/autofit/autofit.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/cache/ftcache.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/gxvalid/gxvalid.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/otvalid/otvalid.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/psaux/psaux.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/pshinter/pshinter.c)
list(APPEND SRC src/main/cpp/Common/freetype/src/psnames/psnames.c)

add_library( # Sets the name of the library.
        epub

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${SRC})


ADD_DEFINITIONS(-DANDROID22 -DUSE_CXIMAGE -DANDROID -DLIBDIR="c" -DBUILDING_LIBICONV -DIN_LIBRARY)
# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)
find_library( # Sets the name of the path variable.
        jnigraphics-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        jnigraphics )
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
add_library(libiconv SHARED IMPORTED)
set_target_properties(libiconv  PROPERTIES IMPORTED_LOCATION D:/workspace/DemoReader/libepub/src/main/jniLibs/${ANDROID_ABI}/libiconv.so)

target_link_libraries( # Specifies the target library.
        epub

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} ${jnigraphics-lib} libiconv )