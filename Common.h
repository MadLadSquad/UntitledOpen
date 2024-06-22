#pragma once

#ifdef UVK_LOG_EXPORT_FROM_LIBRARY
    #ifdef _WIN32
        #ifdef UVK_LIB_COMPILE
            #define UVK_PUBLIC_API __declspec(dllexport)
            #define UVK_PUBLIC_TMPL_API __declspec(dllexport)
        #else
            #define UVK_PUBLIC_API __declspec(dllimport)
            #define UVK_PUBLIC_TMPL_API
        #endif
    #else
        #define UVK_PUBLIC_API
        #define UVK_PUBLIC_TMPL_API
    #endif
#else
    #define UVK_PUBLIC_API
    #define UVK_PUBLIC_TMPL_API
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct UVK_PUBLIC_API UOpen_Filter
{
    const char* name;
    const char* spec;
} UOpen_Filter;

typedef enum UOpen_Status
{
    UOPEN_STATUS_ERROR,
    UOPEN_STATUS_SUCCESS,
    UOPEN_STATUS_CANCELLED
} UOpen_Status;

typedef enum UOpen_PickerOperation
{
    UOPEN_PICK_FILE,
    UOPEN_PICK_MULTIPLE,
    UOPEN_PICK_FOLDER,
    UOPEN_SAVE_FILE,
} UOpen_PickerOperation;


typedef struct UVK_PUBLIC_API UOpen_Result
{
    UOpen_Status status;
    UOpen_PickerOperation operation;
    void* data;
} UOpen_Result;

#ifdef __cplusplus
}
#endif