# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/linuxbrew/.linuxbrew/Cellar/cmake/3.31.6/bin/cmake

# The command to remove a file.
RM = /home/linuxbrew/.linuxbrew/Cellar/cmake/3.31.6/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/nawa/CascadeProjects/windsurf-project

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nawa/CascadeProjects/windsurf-project/build

# Include any dependencies generated for this target.
include CMakeFiles/unicode_example.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/unicode_example.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/unicode_example.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/unicode_example.dir/flags.make

CMakeFiles/unicode_example.dir/codegen:
.PHONY : CMakeFiles/unicode_example.dir/codegen

CMakeFiles/unicode_example.dir/src/main.cpp.o: CMakeFiles/unicode_example.dir/flags.make
CMakeFiles/unicode_example.dir/src/main.cpp.o: /home/nawa/CascadeProjects/windsurf-project/src/main.cpp
CMakeFiles/unicode_example.dir/src/main.cpp.o: CMakeFiles/unicode_example.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/nawa/CascadeProjects/windsurf-project/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/unicode_example.dir/src/main.cpp.o"
	/usr/lib/llvm-18/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/unicode_example.dir/src/main.cpp.o -MF CMakeFiles/unicode_example.dir/src/main.cpp.o.d -o CMakeFiles/unicode_example.dir/src/main.cpp.o -c /home/nawa/CascadeProjects/windsurf-project/src/main.cpp

CMakeFiles/unicode_example.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/unicode_example.dir/src/main.cpp.i"
	/usr/lib/llvm-18/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/nawa/CascadeProjects/windsurf-project/src/main.cpp > CMakeFiles/unicode_example.dir/src/main.cpp.i

CMakeFiles/unicode_example.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/unicode_example.dir/src/main.cpp.s"
	/usr/lib/llvm-18/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/nawa/CascadeProjects/windsurf-project/src/main.cpp -o CMakeFiles/unicode_example.dir/src/main.cpp.s

# Object files for target unicode_example
unicode_example_OBJECTS = \
"CMakeFiles/unicode_example.dir/src/main.cpp.o"

# External object files for target unicode_example
unicode_example_EXTERNAL_OBJECTS =

unicode_example: CMakeFiles/unicode_example.dir/src/main.cpp.o
unicode_example: CMakeFiles/unicode_example.dir/build.make
unicode_example: CMakeFiles/unicode_example.dir/compiler_depend.ts
unicode_example: /home/linuxbrew/.linuxbrew/lib/libboost_locale.so.1.87.0
unicode_example: /home/linuxbrew/.linuxbrew/lib/libboost_chrono.so.1.87.0
unicode_example: /home/linuxbrew/.linuxbrew/lib/libboost_system.so.1.87.0
unicode_example: /home/linuxbrew/.linuxbrew/lib/libboost_thread.so.1.87.0
unicode_example: CMakeFiles/unicode_example.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/nawa/CascadeProjects/windsurf-project/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable unicode_example"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/unicode_example.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/unicode_example.dir/build: unicode_example
.PHONY : CMakeFiles/unicode_example.dir/build

CMakeFiles/unicode_example.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/unicode_example.dir/cmake_clean.cmake
.PHONY : CMakeFiles/unicode_example.dir/clean

CMakeFiles/unicode_example.dir/depend:
	cd /home/nawa/CascadeProjects/windsurf-project/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nawa/CascadeProjects/windsurf-project /home/nawa/CascadeProjects/windsurf-project /home/nawa/CascadeProjects/windsurf-project/build /home/nawa/CascadeProjects/windsurf-project/build /home/nawa/CascadeProjects/windsurf-project/build/CMakeFiles/unicode_example.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/unicode_example.dir/depend

