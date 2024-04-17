#include "UntitledOpen.hpp"
#include "C/CUntitledOpen.h"
#include <nfd.h>

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
#elif __APPLE__
    #include <CoreFoundation/CFBundle.h>
    #include <ApplicationServices/ApplicationServices.h>
#else

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

std::vector<UOpen::UniqueString> UOpen::Result::getPaths() noexcept
{
    std::vector<UniqueString> res;
    if (result.data != nullptr && result.status == UOPEN_STATUS_SUCCESS)
    {
        if (result.operation == UOPEN_PICK_MULTIPLE)
        {
            res.resize(UOpen_getPathCount(&result));
            for (size_t i = 0; i < res.size(); i++)
            {
                char* pt;
                auto r = NFD_PathSet_GetPathU8(result.data, i, &pt);
                if (r == (nfdresult_t)UOPEN_STATUS_SUCCESS)
                {
                    res[i].data = pt;
                    res[i].freeFunc = UOpen_freePathMultiple;
                }
            }
        }
        else
        {
            res.emplace_back();
            res[0].data = (char*)result.data;
            res[0].freeFunc = [](char* d) -> void {
                if (d != nullptr)
                    NFD_FreePath(d);
                d = nullptr;
            };
        }
    }
    return res;
}

UOpen::UniqueString UOpen::Result::getPath(size_t i) const noexcept
{
    if (result.data != nullptr && result.status == UOPEN_STATUS_SUCCESS)
    {
        if (result.operation == UOPEN_PICK_MULTIPLE)
        {
            char* pt;
            auto res = NFD_PathSet_GetPathU8(result.data, i, &pt);
            if (res == (nfdresult_t)UOPEN_STATUS_SUCCESS)
                return {pt, UOpen_freePathMultiple};
            else
                return {};
        }
        else
            return UniqueString{ (const char*)result.data };
    }
    return {};
}

UOpen::Status UOpen::Result::status() const noexcept
{
    return result.status;
}

size_t UOpen::Result::getPathNum() noexcept
{
    return UOpen_getPathCount(&result);
}

UOpen::Result UOpen::pickFile(UOpen::PickerOperation op, const UOpen::Filter* filters, size_t filtersNum, const char* defaultName, const char* defaultPath) noexcept
{
    return Result{ UOpen_pickFile(op, filters, filtersNum, defaultPath, defaultName) };
}

UOpen::Result UOpen::pickFile(UOpen::PickerOperation op, const std::vector<Filter>& filters, const char* defaultName, const char* defaultPath) noexcept
{
    return Result( UOpen_pickFile(op, filters.data(), filters.size(), defaultPath, defaultName) );
}

const char* UOpen::getPickerError() noexcept
{
    return UOpen_getPickerError();
}

UOpen::UniqueString::UniqueString(const char* dt) noexcept
{
    data = (char*)dt;
    freeFunc = [](char* d) -> void {
        if (d != nullptr)
            NFD_FreePath(d);
        d = nullptr;
    };
}

UOpen::UniqueString::UniqueString(const char* dt, UOpen::UniqueString::FreeTypeFunc func) noexcept
{
    data = (char*)dt;
    freeFunc = func;
}

void UOpen::UniqueString::destroy() noexcept
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

void UOpen::openURI(const char* link) noexcept
{
#ifdef _WIN32
    ShellExecuteA(NULL, NULL, link, NULL, NULL, SW_SHOW);
#elif __APPLE__
    CFURLRef url = CFURLCreateWithBytes(NULL, (UInt8*)link, strlen(link), kCFStringEncodingASCII, NULL);
    LSOpenCFURLRef(url, 0);
    CFRelease(url);
#else

#endif
}
