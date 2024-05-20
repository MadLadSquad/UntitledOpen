#pragma once
#include "Common.h"
#include <vector>

namespace UOpen
{
    typedef UOpen_Status Status;
    typedef UOpen_Filter Filter;
    typedef UOpen_PickerOperation PickerOperation;

    // Wrapper class that introduces RAII to strings
    // UntitledImGuiFramework Event Safety - Any time
    class UVK_PUBLIC_API UniqueString
    {
    public:
        UniqueString() = default;
        explicit UniqueString(const char* dt) noexcept;

        operator const char*() const noexcept;

        void destroy() noexcept;
        ~UniqueString() noexcept;
    private:
        friend class Result;
        typedef void(*FreeTypeFunc)(char*);

        UniqueString(const char* dt, FreeTypeFunc func) noexcept;

        FreeTypeFunc freeFunc;
        char* data = nullptr;
    };

    // Picker result class
    // UntitledImGuiFramework Event Safety - Any time
    class UVK_PUBLIC_API Result
    {
    public:
        Result() = default;
        explicit Result(const UOpen_Result& res) noexcept;

        std::vector<UniqueString> getPaths() noexcept;
        UniqueString getPath(size_t i) const noexcept;
        size_t getPathNum() noexcept;
        Status status() const noexcept;
    private:
        UOpen_Result result{};
    };

    // Initialise the library. Call this after creating your window
    // Event Safety - begin, style, post-begin
    UVK_PUBLIC_API void init() noexcept;
    // Free the library. Call this before closing your window
    // Event Safety - begin, style, post-begin
    UVK_PUBLIC_API void destroy() noexcept;

    /**
     * @brief Picks a file, multiple files, folder or saves a file
     * @param op - File operation enum. Depending on the operation the following parameters may or may not be used
     * @param filters - File filters. Example filters value: { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } };
     * This value is ignored when the file operation is UOPEN_PICK_FOLDER
     * @param filtersNum - Number of filters in the filters array
     * @param defaultName - Default name for a folder name. Only used when creating folders. Can be left as ""
     * @param defaultPath - Default path for the dialog. Mostly ignored, for example on Linux. Can be left as NULL
     * @return File pick result struct. Use this to get the strings. Has to be freed with UOpen_freeResult
     * @note Event Safety - begin, style, post-begin
     */
    UVK_PUBLIC_API Result pickFile(PickerOperation op, const Filter* filters, size_t filtersNum, const char* defaultName = "", const char* defaultPath = nullptr) noexcept;
    /**
     * @brief Picks a file, multiple files, folder or saves a file
     * @param op - File operation enum. Depending on the operation the following parameters may or may not be used
     * @param filters - File filters. Example filters value: { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } };
     * This value is ignored when the file operation is UOPEN_PICK_FOLDER
     * @param defaultName - Default name for a folder name. Only used when creating folders. Can be left as ""
     * @param defaultPath - Default path for the dialog. Mostly ignored, for example on Linux. Can be left as NULL
     * @return File pick result struct. Use this to get the strings. Has to be freed with UOpen_freeResult
     * @note Event Safety - begin, style, post-begin
     */
    UVK_PUBLIC_API Result pickFile(PickerOperation op, const std::vector<Filter>& filters, const char* defaultName = "", const char* defaultPath = nullptr) noexcept;

    // Returns an error string if the returned status is UOPEN_STATUS_ERROR
    // Event Safety - begin, style, post-begin
    UVK_PUBLIC_API const char* getPickerError() noexcept;

    /**
     * @brief Opens a URI using the default system application
     * @arg link - URI to open
     * @arg parentWindow - Parent window string. Leave as "" if you don't have a parent window or if you're on Windows or macOS X.
     * More info: https://flatpak.github.io/xdg-desktop-portal/docs/window-identifiers.html
     * @note Event Safety - begin, style, post-begin
     */
    UVK_PUBLIC_API void openURI(const char* link, const char* parentWindow = "") noexcept;
}