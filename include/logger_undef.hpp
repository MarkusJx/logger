/*
 * logger_undef.hpp
 * A file for un-defining all macros defined by logger.hpp
 *
 * Copyright (c) 2021 MarkusJx
 * Licensed under the MIT License
 */
#ifndef LOGGER_LOGGER_UNDEF_HPP
#define LOGGER_LOGGER_UNDEF_HPP

// Un-define all default macros
#undef debug
#undef warning
#undef error

// Un-define all formatted message macros
#undef debugf
#undef warningf
#undef errorf

// Un-define all stream macros
#undef debugStream
#undef warningStream
#undef errorStream

#endif //LOGGER_LOGGER_UNDEF_HPP
