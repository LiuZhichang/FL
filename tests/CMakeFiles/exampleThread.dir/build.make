# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /lzc/backup/FL/tests

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /lzc/backup/FL/tests

# Include any dependencies generated for this target.
include CMakeFiles/exampleThread.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/exampleThread.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/exampleThread.dir/flags.make

CMakeFiles/exampleThread.dir/exampleThread.cpp.o: CMakeFiles/exampleThread.dir/flags.make
CMakeFiles/exampleThread.dir/exampleThread.cpp.o: exampleThread.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/lzc/backup/FL/tests/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/exampleThread.dir/exampleThread.cpp.o"
	/usr/local/share/gcc-10.2/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/exampleThread.dir/exampleThread.cpp.o -c /lzc/backup/FL/tests/exampleThread.cpp

CMakeFiles/exampleThread.dir/exampleThread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/exampleThread.dir/exampleThread.cpp.i"
	/usr/local/share/gcc-10.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /lzc/backup/FL/tests/exampleThread.cpp > CMakeFiles/exampleThread.dir/exampleThread.cpp.i

CMakeFiles/exampleThread.dir/exampleThread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/exampleThread.dir/exampleThread.cpp.s"
	/usr/local/share/gcc-10.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /lzc/backup/FL/tests/exampleThread.cpp -o CMakeFiles/exampleThread.dir/exampleThread.cpp.s

# Object files for target exampleThread
exampleThread_OBJECTS = \
"CMakeFiles/exampleThread.dir/exampleThread.cpp.o"

# External object files for target exampleThread
exampleThread_EXTERNAL_OBJECTS =

/lzc/backup/FL/build/exampleThread: CMakeFiles/exampleThread.dir/exampleThread.cpp.o
/lzc/backup/FL/build/exampleThread: CMakeFiles/exampleThread.dir/build.make
/lzc/backup/FL/build/exampleThread: libFL.so
/lzc/backup/FL/build/exampleThread: CMakeFiles/exampleThread.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/lzc/backup/FL/tests/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /lzc/backup/FL/build/exampleThread"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/exampleThread.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/exampleThread.dir/build: /lzc/backup/FL/build/exampleThread

.PHONY : CMakeFiles/exampleThread.dir/build

CMakeFiles/exampleThread.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/exampleThread.dir/cmake_clean.cmake
.PHONY : CMakeFiles/exampleThread.dir/clean

CMakeFiles/exampleThread.dir/depend:
	cd /lzc/backup/FL/tests && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /lzc/backup/FL/tests /lzc/backup/FL/tests /lzc/backup/FL/tests /lzc/backup/FL/tests /lzc/backup/FL/tests/CMakeFiles/exampleThread.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/exampleThread.dir/depend

