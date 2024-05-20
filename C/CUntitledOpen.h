#pragma once
#include "../Common.h"

#ifdef __cplusplus
extern "C"
{
#endif
    // Initialise the library. Call this after creating your window
    // Event Safety - begin, style, post-begin
    UVK_PUBLIC_API void UOpen_init();
    // Free the library. Call this before closing your window
    // Event Safety - begin, style, post-begin
    UVK_PUBLIC_API void UOpen_destroy();

    /**
     * @brief Picks a file, multiple files, folder or saves a file
     * @param op - File operation enum. Depending on the operation the following parameters may or may not be used
     * @param filters - File filters. Example filters value: { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } };
     * This value is ignored when the file operation is UOPEN_PICK_FOLDER
     * @param filtersNum - Number of filters in the filters array
     * @param defaultPath - Default path for the dialog. Mostly ignored, for example on Linux. Can be left as NULL
     * @param defaultName - Default name for a folder name. Only used when creating folders. Can be left as ""
     * @return File pick result struct. Use this to get the strings. Has to be freed with UOpen_freeResult
     * @note Event Safety - begin, style, post-begin
     */
    UVK_PUBLIC_API UOpen_Result UOpen_pickFile(UOpen_PickerOperation op, const UOpen_Filter* filters, size_t filtersNum, const char* defaultPath, const char* defaultName);
    // Returns an error string if the returned status is UOPEN_STATUS_ERROR
    // Event Safety - begin, style, post-begin
    UVK_PUBLIC_API const char* UOpen_getPickerError();
    // Frees the result struct
    // Event Safety - begin, style, post-begin
    UVK_PUBLIC_API void UOpen_freeResult(UOpen_Result* result);

    // Returns the number of paths when using UOPEN_PICK_MULTIPLE
    // Event Safety - begin, style, post-begin
    UVK_PUBLIC_API size_t UOpen_getPathCount(UOpen_Result* result);
    // Returns a path string given an index when using UOPEN_PICK_MULTIPLE
    // Event Safety - begin, style, post-begin
    UVK_PUBLIC_API const char* UOpen_getPathMultiple(UOpen_Result* result, size_t i);
    // Frees a path when using UOPEN_PICK_MULTIPLE
    // Event Safety - begin, style, post-begin
    UVK_PUBLIC_API void UOpen_freePathMultiple(char* path);

    /**
     * @brief Opens a URI using the default system application
     * @arg link - URI to open
     * @arg parentWindow - Parent window string. Leave as "" if you don't have a parent window or if you're on Windows or macOS X.
     * More info: https://flatpak.github.io/xdg-desktop-portal/docs/window-identifiers.html
     * @return 0 on success, -1 on error. Use getPickerError to get an error message.
     * @note Event Safety - begin, style, post-begin
     */
    UVK_PUBLIC_API int UOpen_openURI(const char* link, const char* parentWindow);
#ifdef __cplusplus
}
#endif
