# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

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
CMAKE_COMMAND = /snap/clion/129/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/129/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/rafal/studia/pwsl/LAB3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/rafal/studia/pwsl/LAB3/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/LAB3.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/LAB3.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/LAB3.dir/flags.make

CMakeFiles/LAB3.dir/main.c.o: CMakeFiles/LAB3.dir/flags.make
CMakeFiles/LAB3.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/rafal/studia/pwsl/LAB3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/LAB3.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/LAB3.dir/main.c.o   -c /home/rafal/studia/pwsl/LAB3/main.c

CMakeFiles/LAB3.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/LAB3.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/rafal/studia/pwsl/LAB3/main.c > CMakeFiles/LAB3.dir/main.c.i

CMakeFiles/LAB3.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/LAB3.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/rafal/studia/pwsl/LAB3/main.c -o CMakeFiles/LAB3.dir/main.c.s

# Object files for target LAB3
LAB3_OBJECTS = \
"CMakeFiles/LAB3.dir/main.c.o"

# External object files for target LAB3
LAB3_EXTERNAL_OBJECTS =

LAB3: CMakeFiles/LAB3.dir/main.c.o
LAB3: CMakeFiles/LAB3.dir/build.make
LAB3: CMakeFiles/LAB3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/rafal/studia/pwsl/LAB3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable LAB3"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/LAB3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/LAB3.dir/build: LAB3

.PHONY : CMakeFiles/LAB3.dir/build

CMakeFiles/LAB3.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/LAB3.dir/cmake_clean.cmake
.PHONY : CMakeFiles/LAB3.dir/clean

CMakeFiles/LAB3.dir/depend:
	cd /home/rafal/studia/pwsl/LAB3/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/rafal/studia/pwsl/LAB3 /home/rafal/studia/pwsl/LAB3 /home/rafal/studia/pwsl/LAB3/cmake-build-debug /home/rafal/studia/pwsl/LAB3/cmake-build-debug /home/rafal/studia/pwsl/LAB3/cmake-build-debug/CMakeFiles/LAB3.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/LAB3.dir/depend

