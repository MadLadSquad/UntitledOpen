#include "UntitledOpen.h"

extern void uopen(const char* ptr);
extern void ureveal(const char* ptr);

void UOpen(const char* url)
{
    uopen(url);
}

void UReveal(const char* url)
{
    ureveal(url);
}