#ifndef __PHOTO_H__
#define __PHOTO_H__

#include "Arduino.h"

typedef struct Photo
{
    size_t length = 0;
    uint8_t *buffer = nullptr;
};

#endif // __PHOTO_H__