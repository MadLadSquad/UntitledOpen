#include "CUntitledOpen.h"
#include <nfd.h>
#include "UntitledOpen.hpp"

void UOpen_init(void* waylandDisplay)
{
    NFD_Init();
    UOpen_updateWaylandDisplay(waylandDisplay);
}

void UOpen_destroy()
{
    NFD_Quit();
}

UOpen_Result UOpen_pickFile(const UOpen_PickerOperation op, const UOpen_Filter* filters, const size_t filtersNum, const char* defaultPath, const char* defaultName, const char* title, const char* acceptLabel, const char* cancelLabel, const UOpen_WindowHandlePlatform windowHandlePlatform, void* windowHandle)
{
    UOpen_Result result = { .operation = op, .data = nullptr };

    switch (op)
    {
        case UOPEN_PICK_MULTIPLE:
            {
                const nfdopendialogu8args_t args{
                    .filterList = reinterpret_cast<const nfdu8filteritem_t*>(filters),
                    .filterCount = static_cast<nfdfiltersize_t>(filtersNum),
                    .defaultPath = defaultPath,
                    .parentWindow =
                    {
                        .type = static_cast<size_t>(windowHandlePlatform),
                        .handle = windowHandle,
                    },
                    .title = title,
                    .acceptLabel = acceptLabel,
                    .cancelLabel = cancelLabel,
                };
                result.status = static_cast<UOpen_Status>(
                    NFD_OpenDialogMultipleU8_With(const_cast<const void**>(&result.data), &args)
                );
            }
            break;
        case UOPEN_PICK_FOLDER:
            {
                const nfdpickfolderu8args_t args{
                    .defaultPath = defaultPath,
                    .parentWindow =
                    {
                        .type = static_cast<size_t>(windowHandlePlatform),
                        .handle = windowHandle,
                    },
                    .title = title,
                    .acceptLabel = acceptLabel,
                    .cancelLabel = cancelLabel,
                };
                result.status = static_cast<UOpen_Status>(
                    NFD_PickFolderU8_With(reinterpret_cast<char**>(&result.data), &args)
                );
            }
            break;
        case UOPEN_SAVE_FILE:
            {
                const nfdsavedialogu8args_t args{
                    .filterList = reinterpret_cast<const nfdu8filteritem_t*>(filters),
                    .filterCount = static_cast<nfdfiltersize_t>(filtersNum),
                    .defaultPath = defaultPath,
                    .defaultName = defaultName,
                    .parentWindow =
                    {
                        .type = static_cast<size_t>(windowHandlePlatform),
                        .handle = windowHandle,
                    },
                    .title = title,
                    .acceptLabel = acceptLabel,
                    .cancelLabel = cancelLabel,
                };
                result.status = static_cast<UOpen_Status>(
                    NFD_SaveDialogU8_With(reinterpret_cast<char**>(&result.data), &args)
                );
            }
            break;
        case UOPEN_PICK_MULTIPLE_FOLDERS:
            {
                const nfdpickfolderu8args_t args{
                    .defaultPath = defaultPath,
                    .parentWindow =
                    {
                        .type = static_cast<size_t>(windowHandlePlatform),
                        .handle = windowHandle,
                    },
                    .title = title,
                    .acceptLabel = acceptLabel,
                    .cancelLabel = cancelLabel,
                };

                result.status = static_cast<UOpen_Status>(
                    NFD_PickFolderMultipleU8_With(const_cast<const void**>(&result.data), &args)
                );
            }
            break;
        default:
            {
                const nfdopendialogu8args_t args{
                    .filterList = reinterpret_cast<const nfdu8filteritem_t*>(filters),
                    .filterCount = static_cast<nfdfiltersize_t>(filtersNum),
                    .defaultPath = defaultPath,
                    .parentWindow =
                    {
                        .type = static_cast<size_t>(windowHandlePlatform),
                        .handle = windowHandle,
                    },
                    .title = title,
                    .acceptLabel = acceptLabel,
                    .cancelLabel = cancelLabel,
                };

                result.status = static_cast<UOpen_Status>(
                    NFD_OpenDialogU8_With(reinterpret_cast<char**>(&result.data), &args)
                );
            }
            break;
    }
    return result;
}

const char* UOpen_getPickerError()
{
    return NFD_GetError();
}

void UOpen_freeResult(UOpen_Result* result)
{
    if (result->data != nullptr && result->status == UOPEN_STATUS_SUCCESS)
    {
        if (result->operation == UOPEN_PICK_MULTIPLE || result->operation == UOPEN_PICK_MULTIPLE_FOLDERS)
            NFD_PathSet_Free(result->data);
        else
            NFD_FreePath(static_cast<char*>(result->data));
        result->data = nullptr;
    }
}

size_t UOpen_getPathCount(const UOpen_Result* result)
{
    size_t count = 0;
    if (result->data != nullptr && result->status == UOPEN_STATUS_SUCCESS)
    {
        if (result->operation == UOPEN_PICK_MULTIPLE || result->operation == UOPEN_PICK_MULTIPLE_FOLDERS)
            NFD_PathSet_GetCount(result->data, reinterpret_cast<nfdpathsetsize_t*>(&count));
        else
            count = 1;
    }
    return count;
}

const char* UOpen_getPathMultiple(const UOpen_Result* result, const size_t i)
{
    char* res;
    if (static_cast<UOpen_Status>(NFD_PathSet_GetPathU8(result->data, i, &res)) != UOPEN_STATUS_SUCCESS)
        return nullptr;
    return res;
}

void UOpen_freePathMultiple(char* path)
{
    if (path != nullptr)
        NFD_PathSet_FreePath(path);
}

int UOpen_openURI(const char* link, const char* parentWindow)
{
    return UOpen::openURI(link, parentWindow);
}

void UOpen_updateWaylandDisplay(void* display)
{
#if !defined(__APPLE__) && !defined(_WIN32)
    NFD_SetWaylandDisplay(static_cast<wl_display*>(display));
#endif
}
