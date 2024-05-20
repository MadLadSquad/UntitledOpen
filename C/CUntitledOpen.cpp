#include "CUntitledOpen.h"
#include <nfd.h>

namespace UOpen
{
    extern void openURI(const char* link, const char* parentWindow) noexcept;
};

void UOpen_init()
{
    NFD_Init();
}

void UOpen_destroy()
{
    NFD_Quit();
}

UOpen_Result UOpen_pickFile(UOpen_PickerOperation op, const UOpen_Filter* filters, size_t filtersNum, const char* defaultPath, const char* defaultName)
{
    UOpen_Result result = { .operation = op, .data = nullptr };

    switch (op)
    {
        case UOPEN_PICK_MULTIPLE:
            result.status = (UOpen_Status)NFD_OpenDialogMultiple((const void**)&result.data, (const nfdu8filteritem_t*)filters, filtersNum, defaultPath);
            break;
        case UOPEN_PICK_FOLDER:
            result.status = (UOpen_Status)NFD_PickFolder((char**)&result.data, defaultPath);
            break;
        case UOPEN_SAVE_FILE:
            result.status = (UOpen_Status)NFD_SaveDialog((char**)&result.data, (const nfdu8filteritem_t*)filters, filtersNum, defaultPath, defaultName);
            break;
        default:
            result.status = (UOpen_Status)NFD_OpenDialog((char**)&result.data, (const nfdu8filteritem_t*) filters, filtersNum, defaultPath);
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
        if (result->operation != UOPEN_PICK_MULTIPLE)
            NFD_FreePath((char*)result->data);
        else
            NFD_PathSet_Free(result->data);
        result->data = nullptr;
    }
}

size_t UOpen_getPathCount(UOpen_Result* result)
{
    size_t count = 0;
    if (result->data != nullptr && result->status == UOPEN_STATUS_SUCCESS)
    {
        if (result->operation == UOPEN_PICK_MULTIPLE)
            NFD_PathSet_GetCount(result->data, (nfdpathsetsize_t*)&count);
        else
            count = 1;
    }
    return count;
}

const char* UOpen_getPathMultiple(UOpen_Result* result, size_t i)
{
    char* res;
    if ((UOpen_Status)NFD_PathSet_GetPath(result->data, i, &res) != UOPEN_STATUS_SUCCESS)
        return nullptr;
    return res;
}

void UOpen_freePathMultiple(char* path)
{
    if (path != nullptr)
        NFD_PathSet_FreePath(path);
    path = nullptr;
}

void UOpen_openURI(const char* link, const char* parentWindow)
{
    UOpen::openURI(link, parentWindow);
}