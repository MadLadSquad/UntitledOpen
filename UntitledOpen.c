#include "UntitledOpen.h"

extern void uopen(const char* ptr);
extern void ureveal(const char* ptr);
extern char umsg_dialogue(const char* title, const char* description, char type);

void UOpen(const char* url)
{
    uopen(url);
}

void UReveal(const char* url)
{
    ureveal(url);
}
