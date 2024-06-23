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
#endif

void UOpen::init() noexcept
{
    UOpen_init();
}

void UOpen::destroy() noexcept
{
    UOpen_destroy();
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
        if (result.operation == UOPEN_PICK_MULTIPLE)
        {
            char* pt;
            const auto res = NFD_PathSet_GetPathU8(result.data, i, &pt);
            if (res == static_cast<nfdresult_t>(UOPEN_STATUS_SUCCESS))
                return {pt, UOpen_freePathMultiple};
            return {};
        }
        return UniqueString{ (const char*)result.data };
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

UOpen::Result UOpen::pickFile(const PickerOperation op, const Filter* filters, const size_t filtersNum, const char* defaultName, const char* defaultPath) noexcept
{
    return Result(UOpen_pickFile(op, filters, filtersNum, defaultPath, defaultName));
}

UOpen::Result UOpen::pickFile(const PickerOperation op, const std::vector<Filter>& filters, const char* defaultName, const char* defaultPath) noexcept
{
    return Result(UOpen_pickFile(op, filters.data(), filters.size(), defaultPath, defaultName));
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
    std::string links = link;
    std::string method = "OpenURI";

    dbus_bool_t bWritable = true;
    dbus_bool_t bAsk = false;

    void* data = (void*)link;
    int fd;

    int linkType = DBUS_TYPE_STRING;

    if (links.starts_with("file://"))
    {
        method = "OpenFile";
        bAsk = true;

        fd = open(links.substr(strlen("file://")).c_str(), O_RDWR);
        data = (void*)((intptr_t)fd); // Convert to intptr_t to silence warning
        linkType = DBUS_TYPE_UNIX_FD;
    }

    DBusError error;
    dbus_error_init(&error);

    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SESSION, &error);

    if (dbus_error_is_set(&error))
    {
        // Print error here for debugging lol
        dbus_error_free(&error);
        return -1;
    }

    DBusMessage* message = dbus_message_new_method_call("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.OpenURI", method.c_str());
    dbus_message_append_args(message, DBUS_TYPE_STRING, &parentWindow, linkType, &data, DBUS_TYPE_INVALID);

    DBusMessageIter root;
    DBusMessageIter pair;
    DBusMessageIter sub;
    DBusMessageIter value;

    static constexpr const char* writable = "writable";
    static constexpr const char* ask = "ask";

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

    DBusPendingCall* pending;
    dbus_connection_send_with_reply(connection, message, &pending, -1);
    dbus_connection_flush(connection);
    dbus_pending_call_block(pending);

    DBusMessage* reply;
    reply = dbus_pending_call_steal_reply(pending);

    if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR)
    {
        // Print error here for debugging lol
        return -1;
    }
#endif
    return 0;
}
