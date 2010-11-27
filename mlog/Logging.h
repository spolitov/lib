#pragma once

#include "Logger.h"
#include "Manager.h"
#include "Output.h"

namespace mlog {

#if !MLOG_NO_LOGGING
#define MLOG_DO_OUT(level, value) \
    do { \
        mlog::Output _mlog_output; \
        logger.outputHeader(_mlog_output.out(), mlog::ll##level); \
        _mlog_output.out() << value; \
        _mlog_output.send(logger.group(), mlog::ll##level, logger.name()); \
    } while(false)

#define MLOG_MESSAGE(level, value) \
    do { \
        if(logger.enabled(mlog::ll##level)) \
            MLOG_DO_OUT(level, value); \
    } while(false)

#define MLOG_MESSAGE2(level1, value1, level2, value2) \
    do { \
        if(logger.enabled(mlog::ll##level1)) \
            MLOG_DO_OUT(level1, value1); \
        else if(logger.enabled(mlog::ll##level2)) \
            MLOG_DO_OUT(level2, value2); \
    } while(false)

#define MLOG_FMESSAGE(level, value) \
    do { \
        mlog::Logger & logger = getLogger(); \
        if(logger.enabled(mlog::ll##level)) \
        { \
            mlog::Output _mlog_output; \
            logger.outputHeader(_mlog_output.out(), mlog::ll##level); \
            _mlog_output.out() << value; \
            _mlog_output.send(logger.group(), mlog::ll##level, logger.name()); \
        } \
    } while(false)
#else
#define MLOG_DO_OUT(level, value) \
    do {} while(false)

#define MLOG_MESSAGE(level, value) \
    do {} while(false)

#define MLOG_MESSAGE2(level1, value1, level2, value2) \
    do {} while(false)

#define MLOG_FMESSAGE(level, value) \
    do {} while(false)
#endif

}
