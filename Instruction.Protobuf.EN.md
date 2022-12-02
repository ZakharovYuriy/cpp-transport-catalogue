Installing Protobuf:<br>

You can download Protobuf from the repository on [GitHub](https://github.com/protocolbuffers/protobuf/releases).<br>
+ Select the protobuf-cpp archive with the source code of the latest version and unpack it on your computer. The source code contains a CMake project.<br>
+ Create the build-debug and build-release folders to build two Protobuf configurations. If you use Visual Studio, one build folder will be enough.<br>
 
 ```
Protobuf is available in packages of some operating systems.
For example, on Debian systems, it can be installed
through the apt package manager, rather than building it yourself. 
In this case, the use of the library will be different,
so it's better to download the source code and build it manually.
```
+ We will create a folder in advance in which we will place the Protobuf package. We'll call it /path/to/protobuf/package.<br>
If you are not building through the IDE, run the following commands in the build-debug folder:<br>

```
cmake <путь к protobuf>/cmake -DCMAKE_BUILD_TYPE=Debug \
      -Dprotobuf_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=/path/to/protobuf/package
cmake --build . 
```
 + Additional parameters:<br>
-Dprotobuf_BUILD_TESTS=OFF — in order not to waste time building tests,<br>
:exclamation:<br>
-DCMAKE_INSTALL_PREFIX=/path/to/protobuf/package — to inform where it will be necessary to create a Protobuf package.<br>
For Visual Studio, the commands are slightly different. The configuration is specified not during generation, but during assembly:<br>

```
cmake <путь к protobuf>/cmake ^
      -Dprotobuf_BUILD_TESTS=OFF ^
      -DCMAKE_INSTALL_PREFIX=/path/to/protobuf/package ^
      -Dprotobuf_MSVC_STATIC_RUNTIME=OFF
cmake --build . --config Debug 
```
Setting the protobuf_MSVC_STATIC_RUNTIME variable when building under Visual Studio is very important. The fact is that Visual Studio has two sets of Runtime libraries - libraries that are compiled with each program being built. If it turns out that two binary files depend on libraries of different sets, you will get a headache due to cryptographic errors during the build of the application.<br>

 + So, the library is ready. Now let's run the magic command:<br>

```
cmake --install . 
```
Under Visual Studio, you need to specify the configuration, since it was not set during generation:<br>

```
cmake --install . --config Debug 
```
And CMake will copy all the necessary files to a pre-prepared location /path/to/protobuf/package.<br>

+ We do the same steps with the Release configuration. We will install it in the same folder. Since the static libraries collected in the Debug configuration have the suffix d, there will be no conflict.<br>
