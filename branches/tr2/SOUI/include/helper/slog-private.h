#pragma once

//定义一组SOUI内部使用的LOG输出宏，"soui"定义LOG ID，"soui-lib"用于定义log的filter.
//用户需要使用LOG可以仿照下面的形式定义，不建议在APP中直接使用下面的宏。

#define SLOG_TRACE( log) LOG_TRACE("soui", "soui-lib", log) 
#define SLOG_DEBUG( log) LOG_DEBUG("soui", "soui-lib", log) 
#define SLOG_INFO ( log) LOG_INFO ("soui", "soui-lib", log)  
#define SLOG_WARN ( log) LOG_WARN ("soui", "soui-lib", log)  
#define SLOG_ERROR( log) LOG_ERROR("soui", "soui-lib", log) 
#define SLOG_ALARM( log) LOG_ALARM("soui", "soui-lib", log) 
#define SLOG_FATAL( log) LOG_FATAL("soui", "soui-lib", log) 


#define SLOGFMTT( fmt, ...) LOGFMT_TRACE("soui", "soui-lib", fmt,  ##__VA_ARGS__)
#define SLOGFMTD( fmt, ...) LOGFMT_DEBUG("soui", "soui-lib", fmt,  ##__VA_ARGS__)
#define SLOGFMTI( fmt, ...) LOGFMT_INFO("soui", "soui-lib", fmt,  ##__VA_ARGS__)
#define SLOGFMTW( fmt, ...) LOGFMT_WARN("soui", "soui-lib", fmt,  ##__VA_ARGS__)
#define SLOGFMTE( fmt, ...) LOGFMT_ERROR("soui", "soui-lib", fmt,  ##__VA_ARGS__)
#define SLOGFMTA( fmt, ...) LOGFMT_ALARM("soui", "soui-lib", fmt,  ##__VA_ARGS__)
#define SLOGFMTF( fmt, ...) LOGFMT_FATAL("soui", "soui-lib", fmt,  ##__VA_ARGS__)
