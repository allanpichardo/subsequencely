
#ifndef SESSION_H
#define SESSION_H

#include "sequencer.h"

typedef enum
{
    SES_CLICK_HELD = 0x01,
} SessionFlags;

void session_draw(Sequencer* sr);

u8 session_handle_press(Sequencer* sr, u8 index, u8 value);

#endif
