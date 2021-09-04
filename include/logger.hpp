/*
 * MIT License
 *
 * Copyright (c) 2021 MarkusJx
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef LOGGER_LOGGER_HPP
#define LOGGER_LOGGER_HPP

#include <string>
#include <functional>
#include <sstream>
#include <ctime>
#include <mutex>
#include <cstring>
#include <memory>
#include <thread>
#include <deque>

#ifdef LOGGER_UNIQUE_DEF
// Write a debug message. Must be called on a logger object or the StaticLogger class.
#define logger_debug(message) _debug(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, message)
// Write a warning message. Must be called on a logger object or the StaticLogger class.
#define logger_warning(message) _warning(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, message)
// Write an error message. Must be called on a logger object or the StaticLogger class.
#define logger_error(...) _error(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)

// Write a formatted debug message. Must be called on a logger object or the StaticLogger class.
#define logger_debugf(fmt, ...) _debugf(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
// Write a formatted warning message. Must be called on a logger object or the StaticLogger class.
#define logger_warningf(fmt, ...) _warningf(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
// Write a formatted error message. Must be called on a logger object or the StaticLogger class.
#define logger_errorf(fmt, ...) _errorf(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

// Get the debug stream. Must be called on a logger object or the StaticLogger class.
#define logger_debugStream _debugStream(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__)
// Get the warning stream. Must be called on a logger object or the StaticLogger class.
#define logger_warningStream _warningStream(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__)
// Get the error stream. Must be called on a logger object or the StaticLogger class.
#define logger_errorStream _errorStream(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__)
#else //LOGER_UNIQUE_DEF
// Write a debug message. Must be called on a logger object or the StaticLogger class.
#define debug(message) _debug(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, message)
// Write a warning message. Must be called on a logger object or the StaticLogger class.
#define warning(message) _warning(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, message)
// Write an error message. Must be called on a logger object or the StaticLogger class.
#define error(...) _error(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)

// Write a formatted debug message. Must be called on a logger object or the StaticLogger class.
#define debugf(fmt, ...) _debugf(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
// Write a formatted warning message. Must be called on a logger object or the StaticLogger class.
#define warningf(fmt, ...) _warningf(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
// Write a formatted error message. Must be called on a logger object or the StaticLogger class.
#define errorf(fmt, ...) _errorf(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

// Get the debug stream. Must be called on a logger object or the StaticLogger class.
#define debugStream _debugStream(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__)
// Get the warning stream. Must be called on a logger object or the StaticLogger class.
#define warningStream _warningStream(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__)
// Get the error stream. Must be called on a logger object or the StaticLogger class.
#define errorStream _errorStream(::markusjx::logging::LoggerUtils::removeSlash(__FILE__), __LINE__, __FUNCTION__)
#endif //LOGER_UNIQUE_DEF

#if __cplusplus >= 201603L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201603L)
#   define LOGGER_MAYBE_UNUSED [[maybe_unused]]
#else
#   define LOGGER_MAYBE_UNUSED
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#   define printf printf_s
#   define fprintf fprintf_s
#   define slash '\\'
#   define LOGGER_WINDOWS
#else
#   define slash '/'
#   undef LOGGER_WINDOWS
#endif

#ifndef LOGGER_WINDOWS
using errno_t = int;
#endif

/**
 * The main logging namespace
 */
namespace markusjx::logging {
    /**
     * The logger mode. Includes no logging, output to a file, output to console and both console and file output
     */
    enum LoggerMode {
        // Log to a file
        MODE_FILE = 0,
        // Log to the console only
        MODE_CONSOLE = 1,
        // Log to a file and the console
        MODE_BOTH = 2,
        // Disable logging
        MODE_NONE = 3
    };

    /**
     * The log level
     */
    enum LogLevel {
        // Disable logging
        NONE = 0,
        // Only log errors
        ERROR = 1,
        // Log errors and warnings
        WARNING = 2,
        // Log everything
        DEBUG = 3
    };

    /**
     * The synchronization mode
     */
    enum SyncMode {
        // The default mode (no synchronization)
        DEFAULT = 0,
        // Synchronize all write operations
        SYNC = 1,
        // Write everything in an extra thread
        ASYNC = 2
    };

    /**
     * A namespace for logging options
     */
    class LoggerOptions {
    public:
        /**
         * The logger time format struct
         */
        typedef struct loggerTimeFormat_s {
            // The format string
            const char *format;
            // The size of the formatted time string in bytes
            unsigned short sizeInBytes;
        } loggerTimeFormat;

        /**
         * Set the time format for the logger
         *
         * @param fmt the format
         */
        LOGGER_MAYBE_UNUSED static void setTimeFormat(loggerTimeFormat fmt);

        /**
         * Set the log format.
         * Please refer to the readme to view available options.
         *
         * @param fmt the format string
         */
        LOGGER_MAYBE_UNUSED static void setLogFormat(const char *fmt);

        /**
         * Format a log message
         *
         * @param file the file the message came from
         * @param line the line the message came from
         * @param method the function name
         * @param logLevel the log level
         * @param message the message to format
         * @return the formatted message
         */
        static std::string formatMessage(const char *file, int line, const char *method, const char *logLevel,
                                         const std::string &message);

        /**
         * The log format.
         * Please use setLogFormat to set this.
         */
        static const char *log_fmt;

        /**
         * The logger time format.
         * Please use etTimeFormat to set this.
         */
        static loggerTimeFormat time_fmt;

    private:
        static void formatOption(std::stringstream &ss, char option, const char *file, int line, const char *method,
                                 const char *logLevel, const std::string &message);
    };

    namespace LoggerUtils {
        /**
        * Gets the current time and date. Source: https://stackoverflow.com/a/10467633
        *
        * @return The current time and date as a string
        */
        std::string currentDateTime();

        /**
         * Remove everything but the file name from a string.
         *
         * @param str the input string
         * @return the file name
         */
        const char *removeSlash(const char *str);

        /**
         * A stream for logging
         */
        class LoggerStream : public std::stringstream {
        public:
            /**
             * Create a logger stream
             *
             * @param callback the function to be called when the stream is destroyed
             * @param mode the logger mode
             * @param disabled whether the stream should be disabled
             */
            explicit LoggerStream(std::function<void(std::string)> callback, LoggerMode mode, bool disabled);

            /**
             * Destroy the logger stream and log the message
             */
            ~LoggerStream() override;

        private:
            std::function<void(std::string)> _callback;
            LoggerMode _mode;
            bool _disabled;
        };
    }

    /**
     * The main logger class
     */
    class Logger {
    public:
        /**
         * A logger constructor
         */
        Logger();

        /**
         * A logger constructor. Usage:
         *
         * <code>
         *    logger::Logger logger(logger::LoggerMode::MODE_FILE, logger::LogLevel::DEBUG, "out.log", "at");
         * </code>
         *
         * @param mode the logger mode
         * @param syncMode the synchronization mode
         * @param lvl the logging level
         * @param fileName the output file name
         * @param fileMode the logger file mode
         */
        explicit Logger(LoggerMode mode, LogLevel lvl = DEBUG, SyncMode syncMode = DEFAULT, const char *fileName = "",
                        const char *fileMode = "at");

        /**
         * Write a debug message.
         * You should use the debug macro. Usage:
         *
         * <code>
         *    logger.debug("Some message");
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param message the message
         */
        void _debug(const char *_file, int line, const char *method, const std::string &message);

        /**
         * Write an error message.
         * You should use the error macro. Usage:
         *
         * <code>
         *    logger.error("Some error message");
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param message the message
         */
        void _error(const char *_file, int line, const char *method, const std::string &message);

        /**
         * Write a error message and append an error
         *
         * @param _file the file the error originated from
         * @param line the line number
         * @param method the function name
         * @param message the error message
         * @param e the exception to append
         */
        void _error(const char *_file, int line, const char *method, std::string message, const std::exception &e);

        /**
         * Write a warning message.
         * You should use the warning macro. Usage:
         *
         * <code>
         *    logger.warning("Some warning message");
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param message the message
         */
        void _warning(const char *_file, int line, const char *method, const std::string &message);

        /**
         * Write a formatted debug message.
         * You should use the debugf macro instead.
         *
         * @tparam Args the argument types
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param fmt the format string
         * @param args the arguments to format
         */
        template<class...Args>
        void _debugf(const char *_file, int line, const char *method, const char *fmt, Args...args) {
            int size = snprintf(nullptr, 0, fmt, args...);
            std::string out(size + 1, '\0');

            snprintf(out.data(), out.size(), fmt, args...);
            out.resize(strlen(out.c_str()));
            this->_debug(_file, line, method, out);
        }

        /**
         * Write a formatted warning message.
         * You should use the warningf macro instead.
         *
         * @tparam Args the argument types
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param fmt the format string
         * @param args the arguments to format
         */
        template<class...Args>
        void _warningf(const char *_file, int line, const char *method, const char *fmt, Args...args) {
            int size = snprintf(nullptr, 0, fmt, args...);
            std::string out(size + 1, '\0');

            snprintf(out.data(), out.size(), fmt, args...);
            out.resize(strlen(out.c_str()));
            this->_warning(_file, line, method, out);
        }

        /**
         * Write a formatted debug message.
         * You should use the errorf macro instead.
         *
         * @tparam Args the argument types
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param fmt the format string
         * @param args the arguments to format
         */
        template<class...Args>
        void _errorf(const char *_file, int line, const char *method, const char *fmt, Args...args) {
            int size = snprintf(nullptr, 0, fmt, args...);
            std::string out(size + 1, '\0');

            snprintf(out.data(), out.size(), fmt, args...);
            out.resize(strlen(out.c_str()));
            this->_error(_file, line, method, out);
        }

        /**
         * Get the debug stream.
         * You should use the debugStream macro. Usage:
         *
         * <code>
         *    logger.debugStream() << "Some message and some hex: " << std::hex << 1234;
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @return the debug stream
         */
        LoggerUtils::LoggerStream _debugStream(const char *_file, int line, const char *method);

        /**
         * Get the warning stream.
         * You should use the warningStream macro. Usage:
         *
         * <code>
         *    logger.warningStream() << "Some warning message and some hex: " << std::hex << 1234;
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @return the warning stream
         */
        LoggerUtils::LoggerStream _warningStream(const char *_file, int line, const char *method);

        /**
         * Get the error stream.
         * You should use the errorStream macro. Usage:
         *
         * <code>
         *    logger.errorStream() << "Some error message and some hex: " << std::hex << 1234;
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @return the error stream
         */
        LoggerUtils::LoggerStream _errorStream(const char *_file, int line, const char *method);

        /**
         * The logger destructor
         */
        ~Logger();

    private:
        class log_message {
        public:
            log_message(const char *level, const char *_file, int line, const char *method, std::string message,
                        LogLevel logLevel, bool to_stderr = false);

            LogLevel logLevel;
            const char *method;
            const char *level;
            const char *_file;
            int line;
            std::string message;
            bool to_stderr;
        };

        void write_log_message(const log_message &message);

        void write_log_impl(const log_message &message);

        FILE *file;
        LoggerMode _mode;
        SyncMode sync;
        LogLevel level;
        std::mutex mtx;
        std::thread writeThread;
        std::deque<log_message> messageQueue;
        bool run;

        void init(const char *fileName, const char *fileMode);
    };

    /**
     * A static logger class
     */
    class StaticLogger {
    public:
        /**
         * Create a new instance of the logger
         */
        LOGGER_MAYBE_UNUSED static void create();

        /**
         * Create a new instance of the logger. Usage:
         *
         * <code>
         *    logger::StaticLogger::create(logger::LoggerMode::MODE_FILE, logger::LogLevel::DEBUG, "out.log", "at");
         * </code>
         *
         * @param mode the logger mode
         * @param lvl the logging level
         * @param fileName the output file name
         * @param fileMode the logger file mode
         */
        LOGGER_MAYBE_UNUSED static void
        create(LoggerMode mode, LogLevel lvl = DEBUG, SyncMode syncMode = DEFAULT, const char *fileName = "",
               const char *fileMode = "at");

        /**
         * Write a debug message.
         * You should use the debug macro. Usage:
         *
         * <code>
         *    logger::StaticLogger::debug("Some message");
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param message the message
         */
        LOGGER_MAYBE_UNUSED static void
        _debug(const char *_file, int line, const char *method, const std::string &message);

        /**
         * Write a error message.
         * You should use the error macro. Usage:
         *
         * <code>
         *    logger::StaticLogger::error("Some error message");
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param message the message
         */
        LOGGER_MAYBE_UNUSED static void
        _error(const char *_file, int line, const char *method, const std::string &message);

        /**
         * Write a error message and append an error
         *
         * @param _file the file the error originated from
         * @param line the line number
         * @param method the function name
         * @param message the error message
         * @param e the exception to append
         */
        static void
        _error(const char *_file, int line, const char *method, const std::string &message, const std::exception &e);

        /**
         * Write a warning message.
         * You should use the warning macro. Usage:
         *
         * <code>
         *    logger::StaticLogger::warning("Some warning message");
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param message the message
         */
        LOGGER_MAYBE_UNUSED static void
        _warning(const char *_file, int line, const char *method, const std::string &message);

        /**
         * Write a formatted debug message.
         * You should use the debugf macro instead.
         *
         * @tparam Args the argument types
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param fmt the format string
         * @param args the arguments to format
         */
        template<class...Args>
        static void _debugf(const char *_file, int line, const char *method, const char *fmt, Args...args) {
            instance->_debugf(_file, line, method, fmt, args...);
        }

        /**
         * Write a formatted warning message.
         * You should use the warningf macro instead.
         *
         * @tparam Args the argument types
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param fmt the format string
         * @param args the arguments to format
         */
        template<class...Args>
        static void _warningf(const char *_file, int line, const char *method, const char *fmt, Args...args) {
            instance->_warningf(_file, line, method, fmt, args...);
        }

        /**
         * Write a formatted error message.
         * You should use the errorf macro instead.
         *
         * @tparam Args the argument types
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @param fmt the format string
         * @param args the arguments to format
         */
        template<class...Args>
        static void _errorf(const char *_file, int line, const char *method, const char *fmt, Args...args) {
            instance->_errorf(_file, line, method, fmt, args...);
        }

        /**
         * Get the debug stream.
         * You should use the debugStream macro. Usage:
         *
         * <code>
         *    logger::StaticLogger::debugStream() << "Some message and some hex: " << std::hex << 1234;
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @return the debug stream
         */
        LOGGER_MAYBE_UNUSED static LoggerUtils::LoggerStream
        _debugStream(const char *_file, int line, const char *method);

        /**
         * Get the warning stream.
         * You should use the warningStream macro. Usage:
         *
         * <code>
         *    logger::StaticLogger::warningStream() << "Some warning message and some hex: " << std::hex << 1234;
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @return the warning stream
         */
        LOGGER_MAYBE_UNUSED static LoggerUtils::LoggerStream
        _warningStream(const char *_file, int line, const char *method);

        /**
         * Get the error stream.
         * You should use the errorStream macro. Usage:
         *
         * <code>
         *    logger::StaticLogger::errorStream() << "Some error message and some hex: " << std::hex << 1234;
         * </code>
         *
         * @param _file the file name
         * @param line the line number
         * @param method the function name
         * @return the error stream
         */
        LOGGER_MAYBE_UNUSED static LoggerUtils::LoggerStream
        _errorStream(const char *_file, int line, const char *method);

        /**
         * Destroy the logger instance
         */
        LOGGER_MAYBE_UNUSED static void reset();

    private:
        static std::unique_ptr<Logger> instance;
    };
}

#ifndef LOGGER_NO_UNDEF
#   ifdef LOGGER_WINDOWS
#       undef printf
#       undef fprintf
#       undef LOGGER_WINDOWS
#   endif

#   undef slash
#endif //LOGGER_NO_UNDEF

#endif //LOGGER_LOGGER_HPP
