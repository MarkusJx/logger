# Logger
A simple C++17 logger.

## Usage
### Create a new instance
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