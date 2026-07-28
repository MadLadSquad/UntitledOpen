#include "UntitledOpen.hpp"
#include "C/CUntitledOpen.h"
#include <string>
#include <string_view>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
#elif __APPLE__
    #include <CoreFoundation/CFBundle.h>
    #include <ApplicationServices/ApplicationServices.h>
#else
    #include <dbus/dbus.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

UOpen::Status UOpen::init(void* waylandDisplay) noexcept
{
    return UOpen_init(waylandDisplay);
}

void UOpen::destroy() noexcept
{
    UOpen_destroy();
}

void UOpen::updateWaylandDisplay(void* display)
{
    UOpen_updateWaylandDisplay(display);
}

namespace
{
    // A single pick's string belongs to the Result, which frees it on destruction, so the wrapper handed to the
    // caller gets a copy of its own. That keeps every UniqueString genuinely unique and outliving its Result
    char* duplicateString(const char* str) noexcept
    {
        const size_t size = strlen(str) + 1;
        auto* res = static_cast<char*>(malloc(size));
        if (res != nullptr)
            memcpy(res, str, size);
        return res;
    }

    void freeDuplicatedString(char* str) noexcept
    {
        free(str);
    }
}

UOpen::Result::Result(const UOpen_Result& res) noexcept
{
    result = res;
}

UOpen::Result::Result(Result&& other) noexcept
{
    result = other.result;
    other.result = {};
}

UOpen::Result& UOpen::Result::operator=(Result&& other) noexcept
{
    if (this != &other)
    {
        UOpen_freeResult(&result);
        result = other.result;
        other.result = {};
    }
    return *this;
}

UOpen::Result::~Result() noexcept
{
    // Nothing else frees the picker's allocation. For a multiple selection this is the path set itself, which the
    // individual path strings handed out by the getters are independent of
    UOpen_freeResult(&result);
}

std::vector<UOpen::UniqueString> UOpen::Result::getPaths() const noexcept
{
    const size_t count = getPathNum();

    std::vector<UniqueString> res;
    res.reserve(count);
    for (size_t i = 0; i < count; i++)
        res.push_back(getPath(i));
    return res;
}

UOpen::UniqueString UOpen::Result::getPath(const size_t i) const noexcept
{
    if (result.data == nullptr || result.status != UOPEN_STATUS_SUCCESS)
        return {};

    if (result.operation == UOPEN_PICK_MULTIPLE || result.operation == UOPEN_PICK_MULTIPLE_FOLDERS)
    {
        // Every path the path set hands out is a fresh allocation, so the wrapper can own it directly
        const char* path = UOpen_getPathMultiple(&result, i);
        if (path == nullptr)
            return {};
        return { path, UOpen_freePathMultiple };
    }

    // A single pick holds exactly one path, so any other index is out of range
    if (i > 0)
        return {};

    char* copy = duplicateString(static_cast<const char*>(result.data));
    if (copy == nullptr)
        return {};
    return { copy, freeDuplicatedString };
}

UOpen::Status UOpen::Result::status() const noexcept
{
    return result.status;
}

size_t UOpen::Result::getPathNum() const noexcept
{
    return UOpen_getPathCount(&result);
}

UOpen::Result UOpen::pickFile(const PickerOperation op, const Filter* filters, const size_t filtersNum, const char* defaultPath, const char* defaultName, const char* title, const char* acceptLabel, const char* cancelLabel, const WindowHandlePlatform windowHandlePlatform, void* windowHandle) noexcept
{
    return Result(UOpen_pickFile(op, filters, filtersNum, defaultPath, defaultName, title, acceptLabel, cancelLabel, windowHandlePlatform, windowHandle));
}

UOpen::Result UOpen::pickFile(const PickerOperation op, const std::vector<Filter>& filters, const char* defaultPath, const char* defaultName, const char* title, const char* acceptLabel, const char* cancelLabel, const WindowHandlePlatform windowHandlePlatform, void* windowHandle) noexcept
{
    return Result(UOpen_pickFile(op, filters.data(), filters.size(), defaultPath, defaultName, title, acceptLabel, cancelLabel, windowHandlePlatform, windowHandle));
}

const char* UOpen::getPickerError() noexcept
{
    return UOpen_getPickerError();
}

UOpen::UniqueString::UniqueString(const char* dt, const FreeTypeFunc func) noexcept
{
    data = const_cast<char*>(dt);
    freeFunc = func;
}

UOpen::UniqueString::UniqueString(UniqueString&& other) noexcept
{
    data = other.data;
    freeFunc = other.freeFunc;
    other.data = nullptr;
    other.freeFunc = [](char*) -> void {};
}

UOpen::UniqueString& UOpen::UniqueString::operator=(UniqueString&& other) noexcept
{
    if (this != &other)
    {
        destroy();
        data = other.data;
        freeFunc = other.freeFunc;
        other.data = nullptr;
        other.freeFunc = [](char*) -> void {};
    }
    return *this;
}

void UOpen::UniqueString::destroy() noexcept
{
    freeFunc(data);
    // Leaving the pointer behind would let a second destroy, or a move assignment onto this object, free it again
    data = nullptr;
    freeFunc = [](char*) -> void {};
}

UOpen::UniqueString::~UniqueString() noexcept
{
    this->destroy();
}

UOpen::UniqueString::operator const char*() const noexcept
{
    return data;
}

#if !defined(_WIN32) && !defined(__APPLE__)
namespace
{
    // Decodes the percent-encoded octets(%XX) of a URI component. Invalid escapes are passed through verbatim
    std::string percentDecode(const std::string_view str) noexcept
    {
        constexpr auto hexDigit = [](const char c) -> int
        {
            if (c >= '0' && c <= '9')
                return c - '0';
            if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;
            if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;
            return -1;
        };

        std::string result;
        result.reserve(str.size());
        for (size_t i = 0; i < str.size(); i++)
        {
            if (str[i] == '%' && i + 2 < str.size())
            {
                const int high = hexDigit(str[i + 1]);
                const int low = hexDigit(str[i + 2]);
                if (high >= 0 && low >= 0)
                {
                    result.push_back(static_cast<char>(high << 4 | low));
                    i += 2;
                    continue;
                }
            }
            result.push_back(str[i]);
        }
        return result;
    }
}
#endif

int UOpen::openURI(const char* link, const char* parentWindow) noexcept
{
    if (link == nullptr)
        return -1;

#ifdef _WIN32
    // The ANSI entry point mangles every URI that isn't representable in the active codepage, so go through UTF-16
    const int wideLength = MultiByteToWideChar(CP_UTF8, 0, link, -1, nullptr, 0);
    if (wideLength <= 0)
        return -1;

    std::wstring wideLink(static_cast<size_t>(wideLength) - 1, L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, link, -1, wideLink.data(), wideLength) <= 0)
        return -1;

    // ShellExecute reports success by returning a value greater than 32
    const auto result = ShellExecuteW(nullptr, nullptr, wideLink.c_str(), nullptr, nullptr, SW_SHOW);
    if (reinterpret_cast<INT_PTR>(result) <= 32)
        return -1;
#elif __APPLE__
    CFURLRef url = CFURLCreateWithBytes(nullptr, reinterpret_cast<const UInt8*>(link), static_cast<CFIndex>(strlen(link)), kCFStringEncodingUTF8, nullptr);
    // Unlike free, CFRelease crashes on a null argument, and a malformed URI gets us exactly that
    if (url == nullptr)
        return -1;

    const OSStatus status = LSOpenCFURLRef(url, nullptr);
    CFRelease(url);

    if (status != noErr)
        return -1;
#else
    // The C API has no default argument, so a null parent is a valid call. DBus aborts the process on a null string
    const char* parent = parentWindow != nullptr ? parentWindow : "";

    const std::string_view links = link;
    const char* method = "OpenURI";

    dbus_bool_t bWritable = false;
    dbus_bool_t bAsk = false;

    int fileFd = -1;
    std::string filePath;

    constexpr std::string_view filePrefix = "file://";
    if (links.starts_with(filePrefix))
    {
        method = "OpenFile";
        bAsk = true;

        const std::string_view rest = links.substr(filePrefix.size());
        const size_t pathStart = rest.find('/');
        if (pathStart == std::string_view::npos)
            return -1;

        // RFC 8089 permits an authority component, but only files on this machine can be handed to the portal as a descriptor
        const std::string_view authority = rest.substr(0, pathStart);
        if (!authority.empty() && authority != "localhost")
            return -1;

        // The path is percent-encoded in a well formed URI, so anything with a space or a non-ASCII character
        // fails to open unless it's decoded first
        filePath = percentDecode(rest.substr(pathStart));

        // The portal only requires a readable descriptor. Prefer read-write so the opened application can save in
        // place, but fall back to read-only rather than failing outright on files we're not allowed to write
        fileFd = open(filePath.c_str(), O_RDWR | O_CLOEXEC);
        if (fileFd >= 0)
            bWritable = true;
        else
            fileFd = open(filePath.c_str(), O_RDONLY | O_CLOEXEC);

        if (fileFd < 0)
            return -1;
    }

    DBusError error;
    dbus_error_init(&error);

    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SESSION, &error);

    if (dbus_error_is_set(&error))
    {
        // Print error here for debugging lol
        dbus_error_free(&error);
        if (fileFd >= 0)
            close(fileFd);
        return -1;
    }
    if (connection == nullptr)
    {
        if (fileFd >= 0)
            close(fileFd);
        return -1;
    }

    DBusMessage* message = dbus_message_new_method_call("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.OpenURI", method);
    if (message == nullptr)
    {
        dbus_connection_unref(connection);
        if (fileFd >= 0)
            close(fileFd);
        return -1;
    }

    // The descriptor has to be appended as an int. Laundering it through a void* only reads the right half of the
    // pointer slot on little-endian machines and is an aliasing violation everywhere
    if (fileFd >= 0)
        dbus_message_append_args(message, DBUS_TYPE_STRING, &parent, DBUS_TYPE_UNIX_FD, &fileFd, DBUS_TYPE_INVALID);
    else
        dbus_message_append_args(message, DBUS_TYPE_STRING, &parent, DBUS_TYPE_STRING, &link, DBUS_TYPE_INVALID);

    DBusMessageIter root;
    DBusMessageIter pair;
    DBusMessageIter sub;
    DBusMessageIter value;

    static constexpr auto writable = "writable";
    static constexpr auto ask = "ask";

    dbus_message_iter_init_append(message, &root);
    dbus_message_iter_open_container(&root, DBUS_TYPE_ARRAY, "{sv}", &pair);

    // Only OpenFile takes this option, and it now has to match the mode the descriptor was actually opened with
    if (fileFd >= 0)
    {
        dbus_message_iter_open_container(&pair, DBUS_TYPE_DICT_ENTRY, nullptr, &sub);
        dbus_message_iter_append_basic(&sub, DBUS_TYPE_STRING, &writable);
            dbus_message_iter_open_container(&sub, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &value);
            dbus_message_iter_append_basic(&value, DBUS_TYPE_BOOLEAN, &bWritable);
            dbus_message_iter_close_container(&sub, &value);
        dbus_message_iter_close_container(&pair, &sub);
    }

    dbus_message_iter_open_container(&pair, DBUS_TYPE_DICT_ENTRY, nullptr, &sub);
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_STRING, &ask);
        dbus_message_iter_open_container(&sub, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &value);
        dbus_message_iter_append_basic(&value, DBUS_TYPE_BOOLEAN, &bAsk);
        dbus_message_iter_close_container(&sub, &value);
    dbus_message_iter_close_container(&pair, &sub);

    dbus_message_iter_close_container(&root, &pair);

    // The portal answers this call with a request handle immediately - any user interaction happens afterwards over
    // that handle - so a reply that never arrives means something is wrong. Blocking forever would wedge the calling
    // thread, which is the UI thread going by this function's event safety
    constexpr int timeoutMilliseconds = 10000;

    DBusPendingCall* pending = nullptr;
    const dbus_bool_t bSent = dbus_connection_send_with_reply(connection, message, &pending, timeoutMilliseconds);

    // The message(with its own duped fd) is now owned by the connection's outgoing queue,
    // so we can drop our references regardless of whether the send succeeded.
    dbus_message_unref(message);
    if (fileFd >= 0)
        close(fileFd);

    if (!bSent || pending == nullptr)
    {
        dbus_connection_unref(connection);
        return -1;
    }

    dbus_connection_flush(connection);
    dbus_pending_call_block(pending);

    DBusMessage* reply = dbus_pending_call_steal_reply(pending);
    dbus_pending_call_unref(pending);

    const bool bError = reply == nullptr || dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR;
    if (reply != nullptr)
        dbus_message_unref(reply);

    // dbus_bus_get hands out a reference to the shared session bus, so ours has to go back
    dbus_connection_unref(connection);

    if (bError)
        return -1;
#endif
    return 0;
}
