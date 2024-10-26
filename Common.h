#pragma once

#ifdef MLS_EXPORT_LIBRARY
    #ifdef _WIN32
        #ifdef MLS_LIB_COMPILE
            #define MLS_PUBLIC_API __declspec(dllexport)
        #else
            #define MLS_PUBLIC_API __declspec(dllimport)
        #endif
    #else
        #define MLS_PUBLIC_API
    #endif
#else
    #define MLS_PUBLIC_API
#endif

#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MLS_PUBLIC_API UOpen_Filter
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
    UOPEN_PICK_MULTIPLE_FOLDERS,
} UOpen_PickerOperation;


typedef struct MLS_PUBLIC_API UOpen_Result
{
    UOpen_Status status;
    UOpen_PickerOperation operation;
    void* data;
} UOpen_Result;

#ifdef __cplusplus
}
#endif