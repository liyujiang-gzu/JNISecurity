# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/${ANDROID_ABI})

# 指定编译器版本
cmake_minimum_required(VERSION 3.4.1)

AUX_SOURCE_DIRECTORY(src/main/cpp/ SOURCE_ROOT)
ADD_SUBDIRECTORY(src/main/cpp/json)
ADD_SUBDIRECTORY(src/main/cpp/util)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library(
        # 生成的 so 库名称
        liyujiang
        # STATIC：静态库，是目标文件的归档文件，在链接其它目标的时候使用
        # SHARED：动态库，会被动态链接，在运行时被加载
        # MODULE：模块库，是不会被链接到其它目标中的插件，但是可能会在运行时使用dlopen-系列的函数动态链接
        SHARED
        # 资源文件，可以多个，资源路径是相对路径，相对于本CMakeLists.txt所在目录
        ${SOURCE_ROOT}
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
find_library(
        android
        log-lib
        log
)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_link_libraries(
        # Specifies the target library.
        liyujiang
        CJsonObject
        CUtils
        # Links the target library to the log library
        # included in the NDK.
        log
        android
        ${log-lib}
)