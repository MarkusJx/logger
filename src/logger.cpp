#include <iostream>
#include <future>

#define LOGGER_NO_UNDEF

#include "logger.hpp"

using namespace markusjx::logging;

LOGGER_MAYBE_UNUSED void LoggerOptions::setTimeFormat(loggerTimeFormat fmt) {
    time_fmt = fmt;
}

LOGGER_MAYBE_UNUSED void LoggerOptions::setLogFormat(const char *fmt) {
    log_fmt = fmt;
}

void LoggerOptions::formatOption(std::stringstream &ss, char option, const char *file, int line, const char *method,
                                 const char *logLevel, const std::string &message) {
    switch (option) {
        case 't':
            ss << LoggerUtils::currentDateTime();
            break;
        case 'f':
            ss << file;
            break;
        case 'l':
            ss << line;
            break;
        case 'M':
            ss << method;
            break;
        case 'p':
            ss << logLevel;
            break;
        case 'm':
            ss << message;
            break;
        case 'n':
            ss << std::endl;
            break;
        case '%':
            ss << '%';
            break;
        default:
            break;
    }
}

std::string LoggerOptions::formatMessage(const char *file, int line, const char *method, const char *logLevel,
                                         const std::string &message) {
    std::stringstream ss;
    const std::string format(log_fmt);

    size_t last = 0;
    while (last < std::string::npos) {
        size_t pos = format.find('%', last);
        if (pos == std::string::npos) {
            ss << format.substr(last);
            break;
        } else {
            ss << format.substr(last, pos - last);

            formatOption(ss, format[pos + 1], file, line, method, logLevel, message);
            last = pos + 2;
        }
    }

    return ss.str();
}

LoggerOptions::loggerTimeFormat LoggerOptions::time_fmt = {"%d-%m-%Y %T", 20};

const char *LoggerOptions::log_fmt = "[%t] [%f:%l] [%p] %m%n";

std::string LoggerUtils::currentDateTime() {
    time_t now = time(nullptr);
    struct tm tm{};

    std::string buf;
    buf.resize(LoggerOptions::time_fmt.sizeInBytes);
#ifdef LOGGER_WINDOWS
    localtime_s(&tm, &now);
#else
    tm = *localtime(&now);
#endif
    strftime(const_cast<char *>(buf.data()), LoggerOptions::time_fmt.sizeInBytes, LoggerOptions::time_fmt.format, &tm);

    buf.resize(strlen(buf.c_str()));
    return buf;
}

const char *LoggerUtils::removeSlash(const char *str) {
    return strrchr(str, slash) + 1;
}

LoggerUtils::LoggerStream::LoggerStream(std::function<void(std::string)> callback, LoggerMode mode, bool disabled)
        : _callback(std::move(callback)), _mode(mode), _disabled(disabled) {
    if (mode == LoggerMode::MODE_NONE || disabled) {
        this->setstate(std::ios_base::failbit); // Discard any input
    }
}

LoggerUtils::LoggerStream::~LoggerStream() {
    if (_mode != LoggerMode::MODE_NONE && !_disabled) {
        _callback(this->str());
    }
}

// Logger class ==========================================

Logger::log_message::log_message(const char *level, const char *_file, int line, const char *method,
                                 std::string message, LogLevel logLevel, bool to_stderr)
        : level(level), _file(_file), line(line), method(method), message(std::move(message)), to_stderr(to_stderr),
          logLevel(logLevel) {}

Logger::Logger() : mtx(), messageQueue(), run(false), writeThread() {
    _mode = MODE_CONSOLE;
    level = LogLevel::DEBUG;
    file = nullptr;
    sync = DEFAULT;

    init(nullptr, nullptr);
}

Logger::Logger(LoggerMode mode, LogLevel lvl, SyncMode syncMode, const char *fileName, const char *fileMode) : mtx(),
                                                                                                               messageQueue(),
                                                                                                               run(false),
                                                                                                               writeThread() {
    _mode = mode;
    level = lvl;
    file = nullptr;
    sync = syncMode;

    if (syncMode == ASYNC) {
        run = true;
        writeThread = std::thread([this] {
            while(run || !messageQueue.empty()) {
                std::unique_lock<std::mutex> lock(mtx);
                if (!messageQueue.empty()) {
                    log_message msg = messageQueue.front();
                    messageQueue.pop_front();
                    lock.unlock();

                    write_log_impl(msg);
                } else {
                    lock.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        });
    }

    init(fileName, fileMode);
}

void Logger::_debug(const char *_file, int line, const char *method, const std::string &message) {
    write_log_message(log_message("DEBUG", _file, line, method, message, DEBUG));
}

void Logger::_error(const char *_file, int line, const char *method, const std::string &message) {
    write_log_message(log_message("ERROR", _file, line, method, message, ERROR, true));
}

void Logger::_error(const char *_file, int line, const char *method, std::string message, const std::exception &e) {
    message = message.append(" ").append(e.what());
    write_log_message(log_message("ERROR", _file, line, method, message, ERROR, true));
}

void Logger::_warning(const char *_file, int line, const char *method, const std::string &message) {
    write_log_message(log_message("WARN", _file, line, method, message, WARNING, true));
}

LoggerUtils::LoggerStream Logger::_debugStream(const char *_file, int line, const char *method) {
    return LoggerUtils::LoggerStream([this, _file, line, method](const std::string &buf) {
        this->_debug(_file, line, method, buf);
    }, _mode, level != DEBUG);
}

LoggerUtils::LoggerStream Logger::_warningStream(const char *_file, int line, const char *method) {
    return LoggerUtils::LoggerStream([this, _file, line, method](const std::string &buf) {
        this->_warning(_file, line, method, buf);
    }, _mode, level != DEBUG && level != WARNING);
}

LoggerUtils::LoggerStream Logger::_errorStream(const char *_file, int line, const char *method) {
    return LoggerUtils::LoggerStream([this, _file, line, method](const std::string &buf) {
        this->_error(_file, line, method, buf);
    }, _mode, level == NONE);
}

void Logger::write_log_message(const log_message &message) {
    if (message.logLevel <= level) {
        if (sync == SYNC) {
            std::unique_lock<std::mutex> lock(mtx);
            write_log_impl(message);
        } else if (sync == ASYNC) {
            std::unique_lock<std::mutex> lock(mtx);
            messageQueue.push_back(message);
        } else {
            write_log_impl(message);
        }
    }
}

void Logger::write_log_impl(const log_message &message) {
    std::string formatted;
    if (_mode != MODE_NONE && level >= message.logLevel) {
        formatted = LoggerOptions::formatMessage(message._file, message.line, message.method,
                                                 message.level, message.message);
    }

    if (file != nullptr && (_mode == MODE_FILE || _mode == MODE_BOTH) && level >= message.logLevel) {
        fprintf(this->file, "%s", formatted.c_str());
    }

    if ((_mode == MODE_BOTH || _mode == MODE_CONSOLE) && level >= message.logLevel) {
        if (message.to_stderr) {
            fprintf(stderr, "%s", formatted.c_str());
        } else {
            printf("%s", formatted.c_str());
        }
    }
}

Logger::~Logger() {
    this->debug("Closing logger");

    if (sync == ASYNC) {
        auto future = std::async(std::launch::async, &std::thread::join, &writeThread);
        run = false;
        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
            std::cerr << "Could not stop the write thread in time, just detaching it" << std::endl;
            writeThread.detach();
        }
    }

    if (file && (_mode == MODE_BOTH || _mode == MODE_FILE)) {
        this->debug("Closing logger file stream");

        errno_t err = fclose(this->file);
        if (err) {
            perror("Could not close logger file stream!");
        }
    }
}

void Logger::init(const char *fileName, const char *fileMode) {
    if (_mode == MODE_BOTH || _mode == MODE_FILE) {
#ifdef LOGGER_WINDOWS
        errno_t err = fopen_s(&file, fileName, fileMode);

        if (err) {
            perror("Could not open out.log file!");
            file = nullptr;
        }
#else
        file = fopen("out.log", fileMode);
                if (file == nullptr) {
                    std::cerr << "Could not open out.log file!" << std::endl;
                }
#endif
    }
}

// StaticLogger class ==========================================

LOGGER_MAYBE_UNUSED void StaticLogger::create() {
    instance = std::make_unique<Logger>();
}

void
StaticLogger::create(LoggerMode mode, LogLevel lvl, SyncMode syncMode, const char *fileName, const char *fileMode) {
    instance = std::make_unique<Logger>(mode, lvl, syncMode, fileName, fileMode);
}

void StaticLogger::_debug(const char *_file, int line, const char *method, const std::string &message) {
    instance->_debug(_file, line, method, message);
}

void StaticLogger::_error(const char *_file, int line, const char *method, const std::string &message) {
    instance->_error(_file, line, method, message);
}

void StaticLogger::_error(const char *_file, int line, const char *method, const std::string &message,
                          const std::exception &e) {
    instance->_error(_file, line, method, message, e);
}

void StaticLogger::_warning(const char *_file, int line, const char *method, const std::string &message) {
    instance->_warning(_file, line, method, message);
}

LoggerUtils::LoggerStream StaticLogger::_debugStream(const char *_file, int line, const char *method) {
    return instance->_debugStream(_file, line, method);
}

LoggerUtils::LoggerStream StaticLogger::_warningStream(const char *_file, int line, const char *method) {
    return instance->_warningStream(_file, line, method);
}

LoggerUtils::LoggerStream StaticLogger::_errorStream(const char *_file, int line, const char *method) {
    return instance->_errorStream(_file, line, method);
}

LOGGER_MAYBE_UNUSED void StaticLogger::reset() {
    instance.reset();
}

std::unique_ptr<Logger> StaticLogger::instance = nullptr;
