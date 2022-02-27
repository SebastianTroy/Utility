#ifndef CHROMETRACING_H
#define CHROMETRACING_H

#include <fmt/core.h>

#include <optional>
#include <map>
#include <chrono>
#include <string>
#include <experimental/source_location>

// #define ENABLE_TRACE

#ifdef ENABLE_TRACE
#define TRACE_FUNC() auto tracer = util::ChromeTracing::StackTracer::Function();
#define TRACE_FUNC_ARGS(args) auto tracer = util::ChromeTracing::StackTracer::Function(args);
#define TRACE_LAMBDA(name) auto tracer = util::ChromeTracing::StackTracer::Lambda(name);
#define TRACE_LAMBDA_ARGS(name, args) auto tracer = util::ChromeTracing::StackTracer::Lambda(name, args);
#define TRACE_SCOPE(name) auto tracer = util::ChromeTracing::StackTracer::Scope(name);
#define TRACE_SCOPE_ARGS(name, args) auto tracer = util::ChromeTracing::StackTracer::Scope(name, args);
#else
#define TRACE_FUNC()
#define TRACE_FUNC_ARGS(args)
#define TRACE_LAMBDA(name)
#define TRACE_LAMBDA_ARGS(name, args)
#define TRACE_SCOPE(name)
#define TRACE_SCOPE_ARGS(name, args)
#endif

namespace util {

/**
 * @brief The ChromeTracing class allows the creation of log files that can be
 * viewed in a chrome browser at "chrome:tracing". This allows for performance
 * tracing of a program, and also graphing of values over time. A very powerful
 * data visualisation tool, with support for stack analysis, as well as event
 * frequency and data value evolution analysis.
 *
 * While this can be used to roughly profile relative performance, a strack-
 * tracer tool would be much better. Instead this can be used to nicely
 * visualise what your program is doing, spot bugs, bottlenecks, unexpected
 * behaviour, etc.
 *
 * See documentation for file format here:
 * https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview#
 */
class ChromeTracing {
public:
    enum class EventType : char {
        DurationBegin = 'B', // "Begin", follow with a matching 'E' event to create a DurationEvent
        DurationEnd =   'E', // "End", should follow a matching 'B' event to create a DurationEvent
        Duration =      'X', // "CompleteEvent", combines a B & E event in one entry
        Instantaneous = 'i', // "Instantaneous" event, an event with no duration
        Counter =       'C', // "Counter", used to track a value over time
        // TODO Object events for tracing object lifetimes? tidy impl would involve polymorphism which would likely change optimisation behaviour in a a big way...
    };

    enum class EventScope : char {
        Global =  'g',
        Process = 'p',
        Thread =  't',
    };

    /**
     * name: (name) The name of the event, as displayed in Trace Viewer
     * cat: (sourceLocation) The event categories. This is a comma separated
     *      list of categories for the event. The categories can be used to hide
     *      events in the Trace Viewer UI.
     * ph: (type) The event type. This is a single character which changes
     *     depending on the type of event being output. The valid values are
     *     listed in the table below. We will discuss each phase type below.
     * ts: (timeStamp) The tracing clock timestamp of the event. The timestamps
     *     are provided at microsecond granularity.
     * tts: (duration) Optional. The thread clock timestamp of the event. The
     *      timestamps are provided at microsecond granularity.
     * pid: (traceSection) The process ID for the process that output this
     *      event.
     * tid: (traceSubSection) The thread ID for the thread that output this
     *      event.
     * args: (args) Any arguments provided for the event. Some of the event
     *       types have required argument fields, otherwise, you can put any
     *       information you wish in here. The arguments are displayed in Trace
     *       Viewer when you view an event in the analysis section.
     */
    struct Event {
        std::string name;
        std::string sourceLocation;
        EventType type;
        std::chrono::steady_clock::time_point timeStamp;
        std::optional<std::chrono::steady_clock::duration> duration;
        std::string traceSection;
        std::string traceSubSection;
        std::optional<std::map<std::string, std::string>> args;
    };

    struct TraceWindow {
        std::string name;
        size_t samplesToCollect;
        std::chrono::steady_clock::time_point startTime;
    };

    class StackTracer {
    public:
        StackTracer(StackTracer&& other);
        ~StackTracer();

        static std::optional<StackTracer> Function(std::optional<std::map<std::string, std::string>> args = std::nullopt, const std::experimental::source_location location = std::experimental::source_location::current());
        static std::optional<StackTracer> Lambda(const std::string& name, std::optional<std::map<std::string, std::string>> args = std::nullopt, const std::experimental::source_location location = std::experimental::source_location::current());
        static std::optional<StackTracer> Scope(const std::string& name, std::optional<std::map<std::string, std::string>> args = std::nullopt, const std::experimental::source_location location = std::experimental::source_location::current());
    private:
        std::string name_;
        std::string sourceLocation_;
        std::string thread_;
        std::optional<std::map<std::string, std::string>> args_;
        bool traceOnExit_ = true;

        StackTracer(std::string name, std::string sourceLocation, std::string thread, std::optional<std::map<std::string, std::string>> args);
    };

    static void AddTraceWindow(std::string name, size_t eventCount, std::chrono::steady_clock::time_point traceStart);
    static void AddEvent(Event&& event);
    static void AddEvent(std::string name, std::string sourceLocation, ChromeTracing::EventType type, std::chrono::steady_clock::time_point timeStamp, std::string traceSection, std::string traceSubSection, std::optional<std::map<std::string, std::string>> args);

private:
    static inline std::string traceDirectory_ = "C:/Users/Troyseph/Desktop/";
    static inline std::vector<TraceWindow> traceWindows_ = {};
    static inline std::vector<Event> events_ = {};

    static std::string ToString(const std::map<std::string, std::string>& pairs);
    static bool IsTracing();
    static void CheckCache();
    static void WriteToFile(std::string fileName, bool append = false);
};

} // namespace util

#endif // CHROMETRACING_H
