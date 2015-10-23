

#include "session.h"

void session_draw(Sequencer* sr)
{
    u8 seq_i;
    u8 linked_seq_i;
    u8 row_seq_i;
    Sequence* s;
    Sequence* linked_seq;

    for (s8 y = GRID_SIZE - 1; y >= 0; y--)
    {
        row_seq_i = row_to_seq(y);

        if (y < GRID_SIZE - 1
            && flag_is_set(linked_seq->flags, SEQ_LINKED_TO))
        {
            linked_seq_i++;
            linked_seq = &s[linked_seq_i];
        }
        else
        {
            seq_i = row_seq_i;
            linked_seq_i = 0;
            s = &sr->sequences[seq_i];
            linked_seq = &s[linked_seq_i];
        }

        u8 copy_blink = modifier_held(LP_DUPLICATE)
            && sr->copied_sequence == row_seq_i
            && linked_seq->playhead % 2 == 0;

        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            u8 step = x * STEPS_PER_PAD + linked_seq_i * SEQUENCE_LENGTH;
            Note* n = sequence_get_note(s, step);
            u8 skip = flag_is_set(n->flags, NTE_SKIP);
            u8 index = coord_to_index(x, y);

            if (copy_blink)
            {
                plot_pad(index, on_color);
            }
            else if (modifier_held(LP_CLICK)
                && x == linked_seq->clock_div - 1)
            {
                plot_pad(index, on_color);
            }
            else if (skip)
            {
                plot_pad(index, off_color);
            }
            else
            {
                u8 linked_playhead = s->playhead - linked_seq_i * SEQUENCE_LENGTH;
                u8 dimness = 5 * (linked_playhead / STEPS_PER_PAD != x);
                plot_pad_dim(index, sequence_colors[seq_i], dimness);
            }
        }
    }
}


u8 session_handle_press(Sequencer* sr, u8 index, u8 value)
{
    // When duplicate is pressed or released, reset the clipboard.
    if (index == LP_DUPLICATE)
    {
        sequencer_copy(sr, -1);
        return 1;
    }

    u8 x, y;
    if (value == 0 || !index_to_pad(index, &x, &y))
    {
        return 0;
    }

    u8 seq_i = row_to_seq(y);
    Sequence* s = &sr->sequences[seq_i];
    u8 shift = modifier_held(LP_SHIFT);
    u8 step = x * STEPS_PER_PAD;

    if (modifier_held(LP_CLICK))
    {
        s->clock_div = x + 1;
    }
    else if (modifier_held(LP_UNDO))
    {
        sequence_reverse(s);
    }
    else if (modifier_held(LP_DELETE))
    {
        for (u8 i = 0; i < STEPS_PER_PAD; i++)
        {
            sequence_clear_note(s, step + i);
        }
    }
    else if (modifier_held(LP_QUANTISE))
    {
        Note* n = sequence_get_note(s, step);
        u8 skip = !flag_is_set(n->flags, NTE_SKIP);
        for (u8 i = 0; i < STEPS_PER_PAD; i++)
        {
            sequence_set_skip(s, step + i, skip);
        }
    }
    else if (modifier_held(LP_DUPLICATE))
    {
        if (sr->copied_sequence == -1)
        {
            sequencer_copy(sr, seq_i);
        }
        else
        {
            sequencer_paste(sr, seq_i);
        }
    }
    else if (modifier_held(LP_DOUBLE))
    {
        if (seq_i < GRID_SIZE - 1)
        {
            Sequence* linked_s = &sr->sequences[seq_i + 1];
            sequence_toggle_linked_to(s);
            sequence_toggle_linked(linked_s);
        }
    }
    else
    {
        sequence_queue_or_jump(s, x * STEPS_PER_PAD, shift);
    }

    return 1;
}
