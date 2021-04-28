# Logger
A simple C++17 logger.

## Usage
### Integrating with a CMake project
If you want to use this project with an existing CMake project,
you may want to use ``ExternalProject_Add`` to build this project:
```Cmake
project(example)
cmake_minimum_required(VERSION 3.15)

# This is required
set(CMAKE_CXX_STANDARD 17)

# Get the ExternalProject module
include(ExternalProject)

# Download and build the logger
ExternalProject_Add(logger_project
                    GIT_REPOSITORY https://github.com/MarkusJx/logger
                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/logger
                    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>)

# Get the install directory path
ExternalProject_Get_Property(logger_project install_dir)
set(logger_dir ${install_dir})
                    
# Include all logger directories
include_directories(${logger_dir}/include)
link_directories(${logger_dir}/lib)

# Create some kind of binary
add_executable(${PROJECT_NAME} test.cpp)

# Link against the static library,
# you may want to link against pthread
# on unix-based operating systems
if (NOT WIN32)
    target_link_libraries(${PROJECT_NAME} logger pthread)
else ()
    target_link_libraries(${PROJECT_NAME} logger)
endif ()

# Add logger as a dependency to the current project
add_dependencies(${PROJECT_NAME} logger_project)
```

### Create a new logger instance
```c++
using namespace markusjx::logging;

// This will create a default logger which
// outputs to the console, log level DEBUG
// and the DEFAULT sync parameter
Logger logger;

// Create a logger with mode both,
// log level WARNING, sync mode DEFAULT,
// and log to a file called 'out.log'
Logger logger(MODE_BOTH, WARNING, DEFAULT, "out.log", "at");

// Create a static logger
StaticLogger::create();
```

NOTE: ``StaticLogger::create()`` only needs to be called once.
After that, the logger can be used anywhere in the program.

### Writing messages
```c++
// Ordinary debug
logger.debug("Some message");

// Static logger version
StaticLogger::debug("Some message");

// Warnings:
logger.warning("Some warning");
StaticLogger::warning("Some warning");

// Errors:
logger.error("Some error");
StaticLogger::error("Some error");

// You may also pass exceptions to the
// error function to print their messages
try {
    // ...
} catch (const std::exception &e) {
    logger.error("An error occurred:", e);
}
```

### Writing formatted messages
It is also possible to write messages with a specified format (like using ``printf``):
```c++
// Will output "Some formetted message"
logger.debugf("Some %s message", "formatted");

// Will output "This is warning 42"
logger.warningf("This is warning %d", 42);

// Will output "This is an error"
logger.errorf(This is %s", "an error");
```

NOTE: This will not affect your message formatting.

### Streams
There are also operators to log messages using streams. The underlying stream is a ``std::stringstream``,
therefore, everything a ``std::stringstream`` supports, is also supported here.
```c++
// Using a debug stream
logger.debugStream << "Using" << ' ' << "streams";

// Using a warning stream
logger.warningStream << "Numbers are also supported: " << 42;

// Using an error stream
logger.errorStream << "You can also write strings: " << std::string("some string");
```
NOTE: There is no need to add a new line to the end of each message,
those will be added to the messages if specified using the [message format](#message-formatting).

## Configuration parameters
### Log level
The following levels can be passed to the logger constructor to set the log level:
* ``DEBUG``: All messages will be logged
* ``WARNING``: Only messages of type ``WARNING`` and ``ERROR`` will be logged
* ``ERROR``: Only messages of type ``ERROR`` will be logged
* ``NONE``: The output will be disabled; No messages will be logged

### Logger mode
The following modes can be passed to the logger constructor in order to set the log mode:
* ``MODE_FILE``: All output will be written to a file
  (The file name must be specified; If no name is specified, nothing will be logged)
* ``MODE_CONSOLE``: All outputs will be redirected to ``stdout`` and ``stderr``
* ``MODE_BOTH``: All outputs will be redirected both to ``stdout`` and ``stderr`` and the specified file
* ``MODE_NONE``: Nothing will be logged

### Synchronisation mode
The following synchronisation modes can be passed to the logger constructor to set the sync mode:
* ``DEFAULT``: No synchronisation will take place. Any output may be overwritten by some other output.
  The logging calls will block until the data has been written.
* ``SYNC``; The logging calls will block until the data has been written. If another logging call takes place
  while the current one hasn't finished, the other thread will wait until the current write operation has finished.
* ``ASYNC``: All data will be written to a queue and then written to the outputs in an extra thread;
  The logging calls will not wait until the data has been written.

## Formatting options
### Message formatting
| Option | Description |
| :---: | :---: |
``%t`` | The current time
``%f`` | The file where the message originated
``%l`` | The line where the message originated
``%M`` | The function name where the message originated
``%p`` | The log level
``%m`` | The message to log
``%n`` | A new line
``%%`` | A literal ``%``

To set the formatter patter, pass your pattern string to ``logging::LoggerOptions::setTimeFormat``:
```c++
logging::LoggerOptions::setTimeFormat("[%t] [%f:%l] [%p] %m%n");
```

By default, all messages are formatted using this pattern: ``"[%t] [%f:%l] [%p] %m%n"``.

### Time formatting
The displayed time is formatted using ``strftime``,
to view available format options, [see here](https://www.cplusplus.com/reference/ctime/strftime/).

To set the time format, you must pass a ``logging::LoggerOptions::loggerTimeFormat``
struct to ``logging::LoggerOptions::setTimeFormat``:
```c++
logging::LoggerOptions::setTimeFormat({
    "%d-%m-%Y %T",  // The format string
    20              // The size of the formatted (output) string in bytes
});
```

By default, dates are formatted using this pattern: ``"%d-%m-%Y %T"``.
