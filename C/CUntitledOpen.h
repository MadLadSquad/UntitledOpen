#pragma once
#include "../Common.h"

#ifdef __cplusplus
extern "C"
{
#endif
    // Initialise the library. Call this after creating your window
    // Set waylandDisplay to a non-null value to set the display on Wayland. Defaults to nullptr
    // Event Safety - begin, style, post-begin
    MLS_PUBLIC_API void UOpen_init(void* waylandDisplay);
    // Free the library. Call this before closing your window
    // Event Safety - begin, style, post-begin
    MLS_PUBLIC_API void UOpen_destroy();

    /**
     * @brief Picks a file, multiple files, folder or saves a file
     * @param op - File operation enum. Depending on the operation the following parameters may or may not be used
     * @param filters - File filters. Example filters value: { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } };
     * This value is ignored when the file operation is UOPEN_PICK_FOLDER
     * @param filtersNum - Number of filters in the filters array
     * @param defaultPath - Default path for the dialog. Mostly ignored, for example on Linux. Can be left as NULL
     * @param defaultName - Default file name. Only used when saving a file. Can be left as ""
     * @param title - The picker window's title
     * @param acceptLabel - The accept button's label. Leave as "" for the default label
     * @param cancelLabel - The cancel button's label. Leave as "" for the default label
     * @param windowHandlePlatform - The backend window platform that your window(if any) and the picker's window run on. Used together with windowHandle to set the parent window for the picker window. Set to NONE by default
     * @param windowHandle - The backend window handle for your window that will be used to parent the picker's window to your window if using a platform type that is not NONE. Set to nullptr by default
     * @return File pick result struct. Use this to get the strings. Has to be freed with UOpen_freeResult
     * @note Event Safety - begin, style, post-begin
     */
    MLS_PUBLIC_API UOpen_Result UOpen_pickFile(
        UOpen_PickerOperation op,
        const UOpen_Filter* filters, size_t filtersNum,
        const char* defaultPath,
        const char* defaultName,
        const char* title,
        const char* acceptLabel,
        const char* cancelLabel,
        UOpen_WindowHandlePlatform windowHandlePlatform, void* windowHandle
    );

    // Returns an error string if the returned status is UOPEN_STATUS_ERROR
    // Event Safety - begin, style, post-begin
    MLS_PUBLIC_API const char* UOpen_getPickerError();
    // Frees the result struct
    // Event Safety - begin, style, post-begin
    MLS_PUBLIC_API void UOpen_freeResult(UOpen_Result* result);

    // Returns the number of paths when using UOPEN_PICK_MULTIPLE
    // Event Safety - begin, style, post-begin
    MLS_PUBLIC_API size_t UOpen_getPathCount(const UOpen_Result* result);
    // Returns a path string given an index when using UOPEN_PICK_MULTIPLE
    // Event Safety - begin, style, post-begin
    MLS_PUBLIC_API const char* UOpen_getPathMultiple(const UOpen_Result* result, size_t i);
    // Frees a path when using UOPEN_PICK_MULTIPLE
    // Event Safety - begin, style, post-begin
    MLS_PUBLIC_API void UOpen_freePathMultiple(char* path);

    // Updates the Wayland display. Set to nullptr to reset the display
    // Event Safety - begin, style, post-begin
    MLS_PUBLIC_API void UOpen_updateWaylandDisplay(void* display);

    /**
     * @brief Opens a URI using the default system application
     * @arg link - URI to open
     * @arg parentWindow - Parent window string. Leave as "" if you don't have a parent window or if you're on Windows or macOS X.
     * More info: https://flatpak.github.io/xdg-desktop-portal/docs/window-identifiers.html
     * @return 0 on success, -1 on error. Use getPickerError to get an error message.
     * @note Event Safety - begin, style, post-begin
     */
    MLS_PUBLIC_API int UOpen_openURI(const char* link, const char* parentWindow);
#ifdef __cplusplus
}
#endif
