#include "ChromeTracing.h"

#include <fstream>
#include <sstream>
#include <thread>
#include <iostream>

namespace util {

void ChromeTracing::AddTraceWindow(std::string name, size_t eventCount, std::chrono::steady_clock::time_point traceStart)
{
    traceWindows_.push_back({ name, eventCount, traceStart });
    std::sort(std::begin(traceWindows_), std::end(traceWindows_), [](const TraceWindow& a, const TraceWindow& b) -> bool
    {
        return a.startTime < b.startTime;
    });
}

void ChromeTracing::AddEvent(ChromeTracing::Event&& event)
{
    // auto now = std::chrono::steady_clock::now();
    if (IsTracing()) {
        if (events_.size() >= traceWindows_.front().samplesToCollect) {
            WriteToFile(traceWindows_.front().name, false);
            traceWindows_.erase(traceWindows_.begin());
            if (!traceWindows_.empty()) {
                events_.reserve(traceWindows_.front().samplesToCollect);
            }
        } else {
            events_.emplace_back(std::move(event));
            // Use below to see if the act  of adding an event to the trace log was taking up the time!
            // std::stringstream idStr;
            // idStr << std::this_thread::get_id();
            // events_.push_back(Event{ "AddEvent", "ChromeTrace:33", EventType::Duration, now, std::chrono::steady_clock::now() - now, "Stack", fmt::format("Thread: {}", idStr.str()), {} });
        }
    }
}

void ChromeTracing::AddEvent(std::string name, std::string sourceLocation, ChromeTracing::EventType type, std::chrono::steady_clock::time_point timeStamp, std::string traceSection, std::string traceSubSection, std::optional<std::map<std::string, std::string>> args)
{
    AddEvent(Event{ name, sourceLocation, type, timeStamp, std::nullopt, traceSection, traceSubSection, std::move(args) });
}

std::string ChromeTracing::ToString(const std::map<std::string, std::string>& pairs)
{
    std::ostringstream ostr;

    ostr << "{ ";
    for (auto& [ key, value ] : pairs) {
        ostr << fmt::format(R"({{ "{}": "{}" }})", key, value);
    }
    ostr << " }";

    return ostr.str();
}

bool ChromeTracing::IsTracing()
{
    return !traceWindows_.empty() && std::chrono::steady_clock::now() >= traceWindows_.front().startTime;
}

ChromeTracing::StackTracer::StackTracer(ChromeTracing::StackTracer&& other)
    : name_(std::move(other.name_))
    , sourceLocation_(std::move(other.sourceLocation_))
    , thread_(std::move(other.thread_))
    , args_(std::move(other.args_))
{
    other.traceOnExit_ = false;
}

ChromeTracing::StackTracer::~StackTracer()
{
    if (traceOnExit_) {
        AddEvent(name_, sourceLocation_, EventType::DurationEnd, std::chrono::steady_clock::now(), "Stack", thread_, args_);
    }
}

std::optional<ChromeTracing::StackTracer> ChromeTracing::StackTracer::Function(std::optional<std::map<std::string, std::string> > args, const std::experimental::source_location location)
{
    std::stringstream idStr;
    idStr << std::this_thread::get_id();
    return StackTracer(location.function_name(), fmt::format("{}:{}", location.file_name(), location.line()), fmt::format("Thread: {}", idStr.str()), args);
}

std::optional<ChromeTracing::StackTracer> ChromeTracing::StackTracer::Lambda(const std::string& name, std::optional<std::map<std::string, std::string> > args, const std::experimental::source_location location)
{
    std::stringstream idStr;
    idStr << std::this_thread::get_id();
    return StackTracer(fmt::format("λ::{}", name), fmt::format("{}:{}", location.file_name(), location.line()), fmt::format("Thread: {}", idStr.str()), args);
}

std::optional<ChromeTracing::StackTracer> ChromeTracing::StackTracer::Scope(const std::string& name, std::optional<std::map<std::string, std::string> > args, const std::experimental::source_location location)
{
    std::stringstream idStr;
    idStr << std::this_thread::get_id();
    return StackTracer(fmt::format("scope::{}", name), fmt::format("{}:{}", location.file_name(), location.line()), fmt::format("Thread: {}", idStr.str()), args);
}

ChromeTracing::StackTracer::StackTracer(std::string name, std::string sourceLocation, std::string thread, std::optional<std::map<std::string, std::string> > args)
    : name_(name)
    , sourceLocation_(sourceLocation)
    , thread_(thread)
    , args_(args)
{
    AddEvent(name, sourceLocation, EventType::DurationBegin, std::chrono::steady_clock::now(), "Stack", thread, args);
}

void ChromeTracing::WriteToFile(std::string fileName, bool append)
{
    std::ofstream fileWriter(traceDirectory_ + fileName + ".trace", append ? std::ofstream::app : std::ofstream::trunc);

    // Start of file needs to include a begin array char and the first event is included to make subsequent events easier to add
    if (!append) {
        fileWriter << fmt::format(R"([ {{ "name" : "{}", "ph" : "{}", "ts" : {}, "pid" : 0 }})",
                                  "TraceStart",
                                  static_cast<char>(EventType::Instantaneous),
                                  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    for (const Event& e : events_) {
        fileWriter << fmt::format(R"(,{} {{ "name" : "{}", "cat" : "{}", "ph" : "{}", "ts" : {}, "pid" : "{}", "tid" : "{}"{}{} }})",
                                  "\n",
                                  e.name,
                                  e.sourceLocation,
                                  static_cast<char>(e.type),
                                  std::chrono::duration_cast<std::chrono::microseconds>(e.timeStamp.time_since_epoch()).count(),
                                  e.traceSection,
                                  e.traceSubSection,
                                  e.duration.has_value() ? fmt::format(R"(, "dur" : {})", std::chrono::duration_cast<std::chrono::microseconds>(e.duration.value()).count()) : "",
                                  e.args.has_value() ? fmt::format(R"(, "args" : {})", ToString(e.args.value())) : "");
    }

    events_.clear();
    std::cout << fmt::format("Trace file {}: {}", append ? "added to" : "created", traceDirectory_ + fileName + ".trace") << std::endl;
}

} // namespace util
