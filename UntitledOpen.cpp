#include "UntitledOpen.hpp"
#include "C/CUntitledOpen.h"
#include <nfd.h>
#include <string>
#include <cstring>

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

void UOpen::init(void* waylandDisplay) noexcept
{
    UOpen_init(waylandDisplay);
}

void UOpen::destroy() noexcept
{
    UOpen_destroy();
}

void UOpen::updateWaylandDisplay(void* display)
{
    UOpen_updateWaylandDisplay(display);
}

UOpen::Result::Result(const UOpen_Result& res) noexcept
{
    result = res;
}

std::vector<UOpen::UniqueString> UOpen::Result::getPaths() const noexcept
{
    std::vector<UniqueString> res;
    if (result.data != nullptr && result.status == UOPEN_STATUS_SUCCESS)
    {
        if (result.operation == UOPEN_PICK_MULTIPLE || result.operation == UOPEN_PICK_MULTIPLE_FOLDERS)
        {
            res.resize(UOpen_getPathCount(&result));
            for (size_t i = 0; i < res.size(); i++)
            {
                char* pt;
                const auto r = NFD_PathSet_GetPathU8(result.data, i, &pt);
                if (r == static_cast<nfdresult_t>(UOPEN_STATUS_SUCCESS))
                {
                    res[i].data = pt;
                    res[i].freeFunc = UOpen_freePathMultiple;
                }
            }
        }
        else
        {
            res.emplace_back();
            res[0].data = static_cast<char*>(result.data);
            res[0].freeFunc = [](char* d) -> void {
                if (d != nullptr)
                    NFD_FreePath(d);
            };
        }
    }
    return res;
}

UOpen::UniqueString UOpen::Result::getPath(const size_t i) const noexcept
{
    if (result.data != nullptr && result.status == UOPEN_STATUS_SUCCESS)
    {
        if (result.operation == UOPEN_PICK_MULTIPLE || result.operation == UOPEN_PICK_MULTIPLE_FOLDERS)
        {
            char* pt;
            const auto res = NFD_PathSet_GetPathU8(result.data, i, &pt);
            if (res == static_cast<nfdresult_t>(UOPEN_STATUS_SUCCESS))
                return {pt, UOpen_freePathMultiple};
            return {};
        }
        return UniqueString{ static_cast<const char*>(result.data) };
    }
    return {};
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

UOpen::UniqueString::UniqueString(const char* dt) noexcept
{
    data = const_cast<char*>(dt);
    freeFunc = [](char* d) -> void {
        if (d != nullptr)
            NFD_FreePath(d);
    };
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

void UOpen::UniqueString::destroy() const noexcept
{
    freeFunc(data);
}

UOpen::UniqueString::~UniqueString() noexcept
{
    this->destroy();
}

UOpen::UniqueString::operator const char*() const noexcept
{
    return data;
}

int UOpen::openURI(const char* link, const char* parentWindow) noexcept
{
#ifdef _WIN32
    ShellExecuteA(NULL, NULL, link, NULL, NULL, SW_SHOW);
#elif __APPLE__
    CFURLRef url = CFURLCreateWithBytes(nullptr, (UInt8*)link, (CFIndex)strlen(link), kCFStringEncodingASCII, nullptr);
    LSOpenCFURLRef(url, 0);
    CFRelease(url);
#else
    const std::string links = link;
    std::string method = "OpenURI";

    constexpr dbus_bool_t bWritable = true;
    dbus_bool_t bAsk = false;

    auto data = const_cast<void*>(reinterpret_cast<const void*>(link));

    int linkType = DBUS_TYPE_STRING;
    int fileFd = -1;

    if (links.starts_with("file://"))
    {
        method = "OpenFile";
        bAsk = true;

        fileFd = open(links.substr(strlen("file://")).c_str(), O_RDWR);
        if (fileFd < 0)
            return -1;

        data = reinterpret_cast<void*>(static_cast<intptr_t>(fileFd)); // Convert to intptr_t to silence warning
        linkType = DBUS_TYPE_UNIX_FD;
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

    DBusMessage* message = dbus_message_new_method_call("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.OpenURI", method.c_str());
    if (message == nullptr)
    {
        if (fileFd >= 0)
            close(fileFd);
        return -1;
    }

    dbus_message_append_args(message, DBUS_TYPE_STRING, &parentWindow, linkType, &data, DBUS_TYPE_INVALID);

    DBusMessageIter root;
    DBusMessageIter pair;
    DBusMessageIter sub;
    DBusMessageIter value;

    static constexpr auto writable = "writable";
    static constexpr auto ask = "ask";

    dbus_message_iter_init_append(message, &root);
    dbus_message_iter_open_container(&root, DBUS_TYPE_ARRAY, "{sv}", &pair);

    dbus_message_iter_open_container(&pair, DBUS_TYPE_DICT_ENTRY, nullptr, &sub);
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_STRING, &writable);
        dbus_message_iter_open_container(&sub, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &value);
        dbus_message_iter_append_basic(&value, DBUS_TYPE_BOOLEAN, &bWritable);
        dbus_message_iter_close_container(&sub, &value);
    dbus_message_iter_close_container(&pair, &sub);

    dbus_message_iter_open_container(&pair, DBUS_TYPE_DICT_ENTRY, nullptr, &sub);
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_STRING, &ask);
        dbus_message_iter_open_container(&sub, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &value);
        dbus_message_iter_append_basic(&value, DBUS_TYPE_BOOLEAN, &bAsk);
        dbus_message_iter_close_container(&sub, &value);
    dbus_message_iter_close_container(&pair, &sub);

    dbus_message_iter_close_container(&root, &pair);

    DBusPendingCall* pending = nullptr;
    const dbus_bool_t bSent = dbus_connection_send_with_reply(connection, message, &pending, -1);

    // The message(with its own duped fd) is now owned by the connection's outgoing queue,
    // so we can drop our references regardless of whether the send succeeded.
    dbus_message_unref(message);
    if (fileFd >= 0)
        close(fileFd);

    if (!bSent || pending == nullptr)
        return -1;

    dbus_connection_flush(connection);
    dbus_pending_call_block(pending);

    DBusMessage* reply = dbus_pending_call_steal_reply(pending);
    dbus_pending_call_unref(pending);

    const bool bError = reply == nullptr || dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR;
    if (reply != nullptr)
        dbus_message_unref(reply);

    if (bError)
        return -1;
#endif
    return 0;
}
