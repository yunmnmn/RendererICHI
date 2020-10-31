#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/Logger.h>
#include <Util/Util.h>

struct RendererTypeInfo
{
   static constexpr const char* GetPrefix()
   {
      return "[Renderer]";
   }

   static constexpr Foundation::Util::Log::Color GetPrefixColor()
   {
      return Foundation::Util::Log::Color::White;
   }
};

#define _LOG_RENDERER(Severity, Message)                                                                                           \
   Foundation::Util::Log::LogTupleWrite<Foundation::Util::Log::ConsoleModule<RendererTypeInfo>,                                    \
                                        Foundation::Util::Log::DebugOutputModule<RendererTypeInfo>>(                               \
       Foundation::Util::Log::LogEntry(Message, Severity, GET_FILE(), GET_FUNC(), GET_LINE()));

#define LOG_WARNING(Message) _LOG_RENDERER(Foundation::Util::Log::Severity::Warning, Message)
#define LOG_ERROR(Message) _LOG_RENDERER(Foundation::Util::Log::Severity::Error, Message)
#define LOG_DEBUG(Message) _LOG_RENDERER(Foundation::Util::Log::Severity::Debug, Message)
#define LOG_INFO(Message) _LOG_RENDERER(Foundation::Util::Log::Severity::Info, Message)

#define VA_ARGS(...) , ##__VA_ARGS__
#define LOG_WARNING_VAR(FORMAT, ...) LOG_WARNING(Foundation::Util::SimpleSprintf(FORMAT VA_ARGS(__VA_ARGS__)).c_str())
#define LOG_ERROR_VAR(FORMAT, ...) LOG_ERROR(Foundation::Util::SimpleSprintf(FORMAT VA_ARGS(__VA_ARGS__)).c_str())
#define LOG_DEBUG_VAR(FORMAT, ...) LOG_DEBUG(Foundation::Util::SimpleSprintf(FORMAT VA_ARGS(__VA_ARGS__)).c_str())
#define LOG_INFO_VAR(FORMAT, ...) LOG_INFO(Foundation::Util::SimpleSprintf(FORMAT VA_ARGS(__VA_ARGS__)).c_str())
