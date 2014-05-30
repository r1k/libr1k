#include "TeletextDecode.h"
#include <iostream>
#include <iomanip>

using namespace std;
namespace libr1k{

    const uint8_t decodeAddress_lookup[] =
    {
        0x00, 0x01, 0x02, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x04, 0x05,
        0x06, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0x08, 0x09, 0x0A, 0x0B,
        0xFF, 0xFF, 0xFF, 0xFF, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF,
        0xFF, 0xFF, 0x10, 0x11, 0x12, 0x13, 0xFF, 0xFF, 0xFF, 0xFF,
        0x14, 0x15, 0x16, 0x17, 0xFF, 0xFF, 0xFF, 0xFF, 0x18, 0x19,
        0x1A, 0x1B, 0xFF, 0xFF, 0xFF, 0xFF, 0x1C, 0x1D, 0x1E, 0x1F,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x21,
        0x22, 0x23, 0xFF, 0xFF, 0xFF, 0xFF, 0x24, 0x25, 0x26, 0x27,
        0xFF, 0xFF, 0xFF, 0xFF, 0x28, 0x29, 0x2A, 0x2B, 0xFF, 0xFF,
        0xFF, 0xFF, 0x2C, 0x2D, 0x2E, 0x2F, 0xFF, 0xFF, 0xFF, 0xFF,
        0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0x34, 0x35,
        0x36, 0x37, 0xFF, 0xFF, 0xFF, 0xFF, 0x38, 0x39, 0x3A, 0x3B,
        0xFF, 0xFF, 0xFF, 0xFF, 0x3C, 0x3D, 0x3E, 0x3F, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };

    const uint8_t hammBytes[] =
    {
        //
        // Hamming decoder. NOTE! this lookup table is only
        // valid when both P and M bits does NOT contain any
        // bit errors.
        //
        0x00, 0x08, 0x00, 0x08, 0x04, 0x0C, 0x04, 0x0C,
        0x00, 0x08, 0x00, 0x08, 0x04, 0x0C, 0x04, 0x0C,
        0x02, 0x0A, 0x02, 0x0A, 0x06, 0x0E, 0x06, 0x0E,
        0x02, 0x0A, 0x02, 0x0A, 0x06, 0x0E, 0x06, 0x0E,
        0x00, 0x08, 0x00, 0x08, 0x04, 0x0C, 0x04, 0x0C,
        0x00, 0x08, 0x00, 0x08, 0x04, 0x0C, 0x04, 0x0C,
        0x02, 0x0A, 0x02, 0x0A, 0x06, 0x0E, 0x06, 0x0E,
        0x02, 0x0A, 0x02, 0x0A, 0x06, 0x0E, 0x06, 0x0E,
        0x01, 0x09, 0x01, 0x09, 0x05, 0x0D, 0x05, 0x0D,
        0x01, 0x09, 0x01, 0x09, 0x05, 0x0D, 0x05, 0x0D,
        0x03, 0x0B, 0x03, 0x0B, 0x07, 0x0F, 0x07, 0x0F,
        0x03, 0x0B, 0x03, 0x0B, 0x07, 0x0F, 0x07, 0x0F,
        0x01, 0x09, 0x01, 0x09, 0x05, 0x0D, 0x05, 0x0D,
        0x01, 0x09, 0x01, 0x09, 0x05, 0x0D, 0x05, 0x0D,
        0x03, 0x0B, 0x03, 0x0B, 0x07, 0x0F, 0x07, 0x0F,
        0x03, 0x0B, 0x03, 0x0B, 0x07, 0x0F, 0x07, 0x0F,
        0x00, 0x08, 0x00, 0x08, 0x04, 0x0C, 0x04, 0x0C,
        0x00, 0x08, 0x00, 0x08, 0x04, 0x0C, 0x04, 0x0C,
        0x02, 0x0A, 0x02, 0x0A, 0x06, 0x0E, 0x06, 0x0E,
        0x02, 0x0A, 0x02, 0x0A, 0x06, 0x0E, 0x06, 0x0E,
        0x00, 0x08, 0x00, 0x08, 0x04, 0x0C, 0x04, 0x0C,
        0x00, 0x08, 0x00, 0x08, 0x04, 0x0C, 0x04, 0x0C,
        0x02, 0x0A, 0x02, 0x0A, 0x06, 0x0E, 0x06, 0x0E,
        0x02, 0x0A, 0x02, 0x0A, 0x06, 0x0E, 0x06, 0x0E,
        0x01, 0x09, 0x01, 0x09, 0x05, 0x0D, 0x05, 0x0D,
        0x01, 0x09, 0x01, 0x09, 0x05, 0x0D, 0x05, 0x0D,
        0x03, 0x0B, 0x03, 0x0B, 0x07, 0x0F, 0x07, 0x0F,
        0x03, 0x0B, 0x03, 0x0B, 0x07, 0x0F, 0x07, 0x0F,
        0x01, 0x09, 0x01, 0x09, 0x05, 0x0D, 0x05, 0x0D,
        0x01, 0x09, 0x01, 0x09, 0x05, 0x0D, 0x05, 0x0D,
        0x03, 0x0B, 0x03, 0x0B, 0x07, 0x0F, 0x07, 0x0F,
        0x03, 0x0B, 0x03, 0x0B, 0x07, 0x0F, 0x07, 0x0F,
    };

    const uint8_t swapBytes[] =
    {
        //
        // LSBF->MSBF translation lookup table
        // or
        // MSBF->LSBF translation lookup table
        //
        0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
        0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
        0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
        0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
        0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
        0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
        0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
        0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
        0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
        0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
        0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
        0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
        0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
        0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
        0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
        0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
        0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
        0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
        0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
        0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
        0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
        0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
        0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
        0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
        0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
        0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
        0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
        0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
        0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
        0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
        0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
        0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
    };

    const uint8_t new_hammBytes[] =
    {
        //
        // Hamming decoder. NOTE! this lookup table corrects bit errors.
        //
        0x01, 0x80, 0x80, 0x08, 0x80, 0x0c, 0x04, 0x80,
        0x80, 0x08, 0x08, 0x08, 0x06, 0x80, 0x80, 0x08,
        0x80, 0x0a, 0x02, 0x80, 0x06, 0x80, 0x80, 0x0f,
        0x06, 0x80, 0x80, 0x08, 0x06, 0x06, 0x06, 0x80,
        0x80, 0x0a, 0x04, 0x80, 0x04, 0x80, 0x04, 0x04,
        0x00, 0x80, 0x80, 0x08, 0x80, 0x0d, 0x04, 0x80,
        0x0a, 0x0a, 0x80, 0x0a, 0x80, 0x0a, 0x04, 0x80,
        0x80, 0x0a, 0x03, 0x80, 0x06, 0x80, 0x80, 0x0e,
        0x01, 0x01, 0x01, 0x80, 0x01, 0x80, 0x80, 0x0f,
        0x01, 0x80, 0x80, 0x08, 0x80, 0x0d, 0x05, 0x80,
        0x01, 0x80, 0x80, 0x0f, 0x80, 0x0f, 0x0f, 0x0f,
        0x80, 0x0b, 0x03, 0x80, 0x06, 0x80, 0x80, 0x0f,
        0x01, 0x80, 0x80, 0x09, 0x80, 0x0d, 0x04, 0x80,
        0x80, 0x0d, 0x03, 0x80, 0x0d, 0x0d, 0x80, 0x0d,
        0x80, 0x0a, 0x03, 0x80, 0x07, 0x80, 0x80, 0x0f,
        0x03, 0x80, 0x03, 0x03, 0x80, 0x0d, 0x03, 0x80,
        0x80, 0x0c, 0x02, 0x80, 0x0c, 0x0c, 0x80, 0x0c,
        0x00, 0x80, 0x80, 0x08, 0x80, 0x0c, 0x05, 0x80,
        0x02, 0x80, 0x02, 0x02, 0x80, 0x0c, 0x02, 0x80,
        0x80, 0x0b, 0x02, 0x80, 0x06, 0x80, 0x80, 0x0e,
        0x00, 0x80, 0x80, 0x09, 0x80, 0x0c, 0x04, 0x80,
        0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x80, 0x0e,
        0x80, 0x0a, 0x02, 0x80, 0x07, 0x80, 0x80, 0x0e,
        0x00, 0x80, 0x80, 0x0e, 0x80, 0x0e, 0x0e, 0x0e,
        0x01, 0x80, 0x80, 0x09, 0x80, 0x0c, 0x05, 0x80,
        0x80, 0x0b, 0x05, 0x80, 0x05, 0x80, 0x05, 0x05,
        0x80, 0x0b, 0x02, 0x80, 0x07, 0x80, 0x80, 0x0f,
        0x0b, 0x0b, 0x80, 0x0b, 0x80, 0x0b, 0x05, 0x80,
        0x80, 0x09, 0x09, 0x09, 0x07, 0x80, 0x80, 0x09,
        0x00, 0x80, 0x80, 0x09, 0x80, 0x0d, 0x05, 0x80,
        0x07, 0x80, 0x80, 0x09, 0x07, 0x07, 0x07, 0x80,
        0x80, 0x0b, 0x03, 0x80, 0x07, 0x80, 0x80, 0x0e,
    };


    bool TtxSubtitlingTask::print_ttx(const uint8_t *b, uint8_t *bff, uint8_t *start_pos, uint8_t *b_col, uint8_t *f_col,
        uint8_t *bg_colours, uint8_t *fg_colours)
    {
        uint8_t i, n, terminator;
        bool double_height;
        uint8_t got_block;
        uint8_t fg_color = 0, bg_color = 0;
        //    uint8_t new_f_col = TTXSUBT_WHITE_FOREGROUND;
        //    uint8_t new_b_col = TTXSUBT_TRANS_BACKGROUND;
        uint8_t new_f_col = WHITE;
        uint8_t new_b_col = BLACK;
        // Use default colors
        *b_col = BLACK;       // Black until non-sensoring character is found
        *f_col = WHITE;       // White until non-sensoring character is found

        double_height = false;
        *start_pos = 0;
        terminator = 0;
        got_block = 0;
        n = 0;

        // For all payload do...
        for (i = 0; i < 40; i++)
        {
            uint8_t ch = *b++;
            // All bytes are coded swapped msb<-->lsb, and protected with parity
            ch = swapBytes[ch] & 0x7f;

            // Colors are coded with control char 1-7 (0 = Black)
            if (ch <= 7)
            {
                // We support only one color pr line, and if there's more than one color use default
                fg_color++;
                new_f_col = ch;
            }
            else if (ch == BLACK_BG)
            {
                // Resetting bg colour to black
                bg_color++;
                new_b_col = BLACK;
            }
            else if (ch == NEW_BG)
            {
                // Setting current fg colour to active bg colour
                bg_color++;
                new_b_col = new_f_col;
            }

            // Check if we got a start command
            if (ch == START_BOX)
            {
                got_block++;
                if (got_block == 2)
                {
                    // Only when two following START_BOX we have the start position
                    if (!(*start_pos))
                    {
                        *start_pos = n;
                    }
                    // Make sure we end the string
                    terminator = 40;
                }
            }
            else if (got_block == 1)
            {
                got_block--;
            }

            // A double height is always (!) signaled for subtitles.
            if (ch == DOUBLE_HEIGHT)
            {
                double_height = true;
            }

            // Write the character to the string, all control chars are replaced with space
            if (got_block > 1)
            {
                bff[n] = (ch < ' ') ? ' ' : ch;
                fg_colours[n] = new_f_col;
                bg_colours[n] = new_b_col;

                // If non-sensuring character found, revert to default colour
                switch (bff[n])
                {
                case ' ':
                case '#':
                case 127: // Block character
                    break;
                default:
                    *b_col = bgClr;
                    *f_col = fgClr;
                }
            }
            n++;

            // This will end the string
            if (ch == END_BOX)
            {
                if (got_block > 1)
                {
                    terminator = n;
                }
                got_block = 0;
            }
        }

        // Terminate the string
        bff[terminator] = 0;

        // If non-spaces detected, bg colour array BLACK reverts to default
        if (*b_col == bgClr)
        {
            for (i = 0; i < 40; i++)
            {
                if (bff[i] == 0)
                {
                    break;
                }
                if (bg_colours[i] == BLACK)
                {
                    bg_colours[i] = bgClr;
                }
            }
        }
        // Retrun size of subtiles
        return double_height;
    }

    void TtxSubtitlingTask::filter_text(const uint8_t *const ttx_local)
    {
        uint8_t start_pos;
        unsigned int row_mag, clr;
        unsigned int eTrip_Address;
        static decoded_triplet dTrip[15];
        static int extended_info_count = 0;
        static int used_flag = -1;
        int pos = 7;

        char dummy, buffer[41];
        uint8_t fg_cols[40], bg_cols[40];

        if (used_flag == -1)
        {
            extended_info_count = 0;
            //@warning memset
            memset((void *)dTrip, 0, sizeof(decoded_triplet)* 15);
            used_flag = 0;
        }

        // First check that we've got subtiles and that length is correct
        if (ttx_local[0] == 0x03 && ttx_local[1] == 0x2C)
        {
            row_mag = 0xffff; // Default to error
            // Get magazine and packet adress  10.1
            dummy = new_hammBytes[ttx_local[5]];
            if ((unsigned char)dummy != 0x80)
            {
                row_mag = (unsigned int)dummy << 4;
                dummy = new_hammBytes[ttx_local[4]];
                if ((unsigned char)dummy != 0x80)
                {
                    row_mag |= (unsigned int)dummy;
                    row_mag &= 0xF8;
                    row_mag >>= 3;
                }
            }

            // Check if we have a X/0 packet
            if (row_mag == 0)
            {
                dummy = new_hammBytes[ttx_local[9]];
                if ((unsigned char)dummy != 0x80)
                {
                    clr = dummy;
                }
                else
                {
                    // Clear to be sure
                    clr = CTL_C4;
                }
                dummy = new_hammBytes[ttx_local[13]];
                if ((unsigned char)dummy != 0x80)
                {
                    // b2 b1 b0 = C14 C13 C12 See paragraph 15.1.2
                    uint8_t language = (dummy >> 1) & 0x07;

                    // Some charsets is selected based on selected language
                    uint8_t sLang = NO_SPECIAL_LANG;

                    // Select character set to be used for this subtitles
                    switch (language)
                    {
                    case 0x00:
                        lang = SCR_LANGUAGE_ENGLISH;
                        break;
                    case 0x01:
                        lang = SCR_LANGUAGE_FRENCH;
                        break;
                    case 0x02:
                        // Norwegian and Danish are signaled as swedish. Use selected
                        // language to decide proper charset.
                        switch (sLang)
                        {
                        case SPECIAL_LANG_NOR:
                        case SPECIAL_LANG_DAN:
                            lang = SCR_LANGUAGE_NORWEGIAN;
                            break;
                        default:
                            lang = SCR_LANGUAGE_SWEDISH;
                        }
                        break;
                    case 0x04:
                        lang = SCR_LANGUAGE_GERMAN;
                        break;
                    case 0x05:
                        lang = SCR_LANGUAGE_SPANISH;
                        break;
                    case 0x06:
                        lang = SCR_LANGUAGE_ITALIAN;
                        break;
                    default:
                        lang = SCR_LANGUAGE_DEFAULT;
                        break;
                    }

                }
                else
                {
                    // Default to english, or keep current?
                }

            }
            else if (row_mag <= 23)
            {
                uint8_t f_col, b_col;
                // Print subtitles to buffer, get start position in string and colors
                uint8_t dblSize = print_ttx(&ttx_local[6], (uint8_t*)buffer, &start_pos, &b_col, &f_col, bg_cols, fg_cols);
                //    LogEvent(EVG_TXT_DRIVER, EVP_NOTICE, "@%02d,%d String \"%s\" colour %d on %d",row_mag, start_pos, (char*)(&buffer[start_pos]),
                //                                       f_col, b_col);
                if ((buffer[start_pos] != 0) || (start_pos != 0))
                {
                    for (int k = 0; k < extended_info_count; k++)
                    {
                        if (dTrip[k].line_num == row_mag)
                        {
                            dTrip[k].character_to_update = dTrip[k].address - start_pos;
                        }
                    }
                    // Create a SubtitleLine which is a region at correct position, and with the
                    // subtitle string redered into it
                    SubtitleLine* lineX;

                    // Lower task priority when creating subtitle line (very CPU intensive)       
                    lineX = new SubtitleLine((char*)buffer, start_pos, row_mag, b_col, f_col, 1, dblSize, lang, extended_info_count, dTrip);
                    if (lineX == 0)
                    {
                        //LogMessage(EVG_TXT_DRIVER, EVP_ERROR, "TTXTSUB: new failed");
                    }


                }
                // else LogEvent(EVG_TXT_DRIVER, EVP_NOTICE, "ignored");
            }
            else if (row_mag == 26)
            {
                extended_info_count = 0;
                used_flag = 0;
                int current_line = 0;
                //@warning memset
                memset((void *)dTrip, 0, sizeof(decoded_triplet)* 15);
                while (dTrip[extended_info_count].address != 0x3F && dTrip[extended_info_count].mode != 0x1F && pos < 44)
                {
                    dTrip[extended_info_count].address = 0;
                    dTrip[extended_info_count].data = 0;
                    dTrip[extended_info_count].mode = 0;

                    eTrip_Address = ttx_local[pos] << 2;
                    eTrip_Address |= (ttx_local[pos + 1] >> 6 & 0x3);
                    eTrip_Address &= 0xBB;
                    dTrip[extended_info_count].address = decodeAddress_lookup[eTrip_Address];
                    if (dTrip[extended_info_count].address == 0xFF)
                    {
                        //TC_printf("ERROR: INVALID VALUE RETURNED FROM decodeAddress_lookup\n");
                    }

                    dTrip[extended_info_count].mode = (ttx_local[pos + 1] >> 1) & 0x1F;
                    dTrip[extended_info_count].data = ttx_local[pos + 2] >> 1;
                    //Swap bit order
                    dTrip[extended_info_count].mode = swapBytes[dTrip[extended_info_count].mode] >> 3;
                    dTrip[extended_info_count].data = swapBytes[dTrip[extended_info_count].data] >> 1;
                    dTrip[extended_info_count].address = swapBytes[dTrip[extended_info_count].address] >> 2;
                    if (dTrip[extended_info_count].address <= 39)
                    {
                        dTrip[extended_info_count].character_to_update = -1;
                        dTrip[extended_info_count].standard_char = dTrip[extended_info_count].data;
                        dTrip[extended_info_count].symbol_offset = dTrip[extended_info_count].mode & 0xF;
                        dTrip[extended_info_count].line_num = current_line;
                        extended_info_count++;
                    }
                    else if (dTrip[extended_info_count].mode == 4)
                    {
                        current_line = dTrip[extended_info_count].address - 40;
                        if (current_line > used_flag) used_flag = current_line;
                    }
                    pos = pos + 3;
                }
            }
        }
    }

    void TtxSubtitlingTask::TtxtGetPage(const uint8_t *data, uint8_t &magazine, uint8_t &row) const
    {
        uint8_t mag = 0x7f; // Default to error (not 0xff because ttx_sub_mag may be 0xff)
        uint8_t ro = 0xff; // Default to error
        uint8_t dummy;

        dummy = new_hammBytes[data[4]];
        if (dummy != 0x80)
        {
            ro = dummy;
            mag = ro & 0x07;
            ro >>= 3;
            dummy = new_hammBytes[data[5]];
            if (dummy != 0x80)
            {
                ro |= dummy << 1;
            }
            else
            {
                mag = 0x7f;
                ro = 0xff;
            }
        } // if (dummy != 0x80)

        magazine = mag;
        row = ro;
    }

    uint16_t TtxSubtitlingTask::_GetSubtPage(const uint8_t *data, const uint8_t magazine, const uint8_t row)
    {
        uint8_t dummy;
        uint16_t page_num = 0xffff; // Default to error

        if (row == 0)
        {
            //
            // X/0 packet
            //

            dummy = new_hammBytes[data[6]];
            if (dummy < 0x0a)
            {
                page_num = (uint16_t)dummy;
                dummy = new_hammBytes[data[7]];
                if (dummy < 0x0a)
                {
                    page_num += (uint16_t)dummy * 10;
                    if (magazine != 0)
                    {
                        // For magazines 1-7
                        page_num += (uint16_t)(magazine)* 100;
                    }
                    else
                    {
                        // Magazine 0 is page numbers 800 - 899
                        page_num += (uint16_t)(8) * 100;
                    }
                }
                else
                {
                    page_num = 0xffff;
                }
            }
        }

        return page_num;
    }

    uint8_t TtxSubtitlingTask::Check_subtitling_field(const uint8_t *const subtitling_field)
    {
        static uint8_t match;
        uint8_t magazine = 0x7f; // Default to error (not 0xff because ttx_sub_mag may be 0xff)
        uint8_t row = 0xff; // Default to error
        uint8_t dummy;

        dummy = new_hammBytes[subtitling_field[4]];
        if (dummy != 0x80)
        {
            row = dummy;
            magazine = row & 0x07;
            row >>= 3;
            dummy = new_hammBytes[subtitling_field[5]];
            if (dummy != 0x80)
            {
                row |= dummy << 1;
            }
            else
            {
                magazine = 0x7f;
                row = 0xff;

            }
        } // if (dummy != 0x80)

        if (ttx_sub_mag == 0xFF)
        {
            match = 0;
        }

        if ((ttx_sub_mag == magazine) || subtPage == ALL_PAGES)
        {
            if (row == 0)
            {
                //
                //X/0 packet
                //
                uint16_t page_num = 0xffff; // Default to error

                dummy = new_hammBytes[subtitling_field[6]];
                if (dummy < 0x0a)
                {
                    page_num = dummy;
                    dummy = new_hammBytes[subtitling_field[7]];
                    if (dummy < 0x0a)
                    {
                        page_num += dummy * 10;
                        if (magazine != 0)
                        {
                            // For magazines 1-7
                            page_num += magazine * 100;
                        }
                        else
                        {
                            // Magazine 0 is page numbers 800 - 899
                            page_num += 8 * 100;
                        }
                    }
                    else
                    {
                        page_num = 0xffff;
                    }
                }
                match = true; // (page_num == subtPage);
            }

            if (match && (ttx_sub_mag == magazine || subtPage == ALL_PAGES))
            {
                return true;    // Valid subtitling field found
            }
        } //   if (ttx_sub_mag == magazine)

        return false;
    }

    void TtxSubtitlingTask::ParseTtxData(const uint8_t *const tdata)
    {
        // A byte ptr is easier to work with. Make sure it is alligned !
        const uint8_t *data = tdata;
        // Data starts here
        data += 8;

        // First of all, we need a valid page
        if ((subtPage < ILLEGAL_TTX_SUB_PAGE) || (subtPage == ALL_PAGES))
        {
            // Check Framing Code 9.2.
            if (data[3] == 0xe4)
            {
                // Check that this is a valid subtitling field
                if (Check_subtitling_field(&data[0]))
                {
                    // Do some processing
                    //LogEvent(EVG_TXT_DRIVER, EVP_NOTICE, "Got valid data");
                    filter_text(&data[0]);
                }
                // The rest of the function is for debug
            }
        }
    }

    void TtxSubtitlingTask::Decode_data_block(const uint8_t *data, string &string_represenation)
    {
        currentString = &string_represenation;

        uint8_t magazine = 0x7f; // Default to error (not 0xff because ttx_sub_mag may be 0xff)
        uint8_t row = 0xff; // Default to error
        uint16_t page_num = 0xffff; // Default to error

        TtxtGetPage(data, magazine, row);

        if (magazine != 0x7F && row == 0)
        {
            if ((_GetSubtPage(data, magazine, row) == subtPage) || subtPage == ALL_PAGES) // is it the page we want
            {
                ParseTtxData(data);
            }
        }
    }

    TeletextPacketHandler::TeletextPacketHandler(ofstream *str, ofstream *txtstream, unsigned int page, bool Debug_on)
        :
        outStream(*str),
        decStream(*txtstream),
        processor(page)
    {
        stream_id = 0xbdu;

        FrameCount = 0;

        DebugOn = Debug_on;
        if (DebugOn)
        {
            LogFile = make_shared<Log>();
        }
        else
        {
            LogFile = nullptr;
        }
    }

    int TeletextPacketHandler::DecodeFrame()
    {
        /*
            data_unit_id     8 uimsbf
            data_unit_length 8 uimsbf
            data_field()

            data_field(){
            reserved_future_use         2   bslbf
            field_parity                1   bslbf
            line_offset                 5   uimsbf
            framing_code                8   bslbf
            magazine_and_packet_address 16  bslbf
            data_block                  320 bslbf
            }
            */
        const int data_field_size = 44;
        const int block_length = data_field_size + 2;
        if (buffer.size() < block_length)
        {
            buffer.clear();
            return 0;
        }

        const int data_unit_id = buffer[0];
        const int data_unit_length = buffer[1];

        if ((data_unit_id != 0x02 || data_unit_id != 0x03) && data_unit_length != 0x2c)
        {
            buffer.clear();
            return 0;
        }

        outStream << data_unit_id << "," << data_unit_length << ",";

        const uint8_t *pData_field = &buffer[2];
        const int field_parity = (pData_field[0] & 0x20) >> 5;
        const int line_offset = pData_field[0] & 0x1f;
        const int framing_code = pData_field[1];
        const int magazine_and_packet_address = pData_field[2] << 8 | pData_field[3];
        outStream << field_parity << "," << line_offset << "," << framing_code << "," << magazine_and_packet_address;

        if (framing_code != 0xe4)
        {
            buffer.clear();
            return 0;
        }

        decStream << field_parity << "," << line_offset << "," << framing_code << "," << magazine_and_packet_address;



        // Now process data block
        string string_representation;

        processor.Decode_data_block(pData_field, string_representation);

        decStream << string_representation << endl;

        // output plain bytes
        outStream << showbase << internal << setfill('0');
        for (int i = 4; i < data_field_size; i++)
        {
            outStream << " " << pData_field[i];
        }
        outStream << endl;

        // remove the used data and return the num bytes used
        buffer.remove(block_length);
        return block_length;
    }

    void TeletextPacketHandler::PESDecode(PESPacket_t *buf)
    {
        uint8_t *PES_data = buf->GetPESData();
        // added to the first byte after the PES header length field
        // Need to adjust PESPacketSize to make it just the payload size
        unsigned int PES_data_size = buf->GetPESDataLength();

        if (!PES_data_size)
            return;

        try
        {
            const int data_identifier = PES_data[0];
            buffer.set(&PES_data[1], PES_data_size - 1);

            while (buffer.size())
            {
                outStream << buf->PTS << ',' << data_identifier << ',';

                DecodeFrame();
            }
            outStream << endl;
        }
        catch (...)
        {
            outStream << endl;
            buffer.clear();
        }
    }


    SubtitleLine::SubtitleLine(char *string, uint8_t start_pos, uint8_t line, uint8_t bg_col, uint8_t fg_col,
        uint8_t Box, uint8_t size, SCR_TXT_FontLanguage_t lang, int ECCount, decoded_triplet *dTrip) :
        lineNo(line), dblSize(size), fg_color(fg_col), bg_color(bg_col), box(Box)
    {
        int t_size = 0;
        FontLanguage_t t_lang;

        // Copy string for later use (only used in debug printing...)
        str = new char[sizeof(char)*strlen(&string[start_pos]) + 1];
        if (str == 0)
        {
            //LogEvent(EVG_TXT_DRIVER, EVP_ERROR, "SUBTLINE: RAM_Malloc failed");
        }

        strcpy(str, &string[start_pos]);

        //LogEvent(EVG_TXT_DRIVER, EVP_DEBUG, "DEBUG: line %d: %s", line, str);

        // Calculate vertical position from line number (map from 625 pixel lines to 24 charachter lines)
        //int y = CalculateVertPos() - 20;
        //out_chars("SubtitleLine::start_pos: %d x: %d",start_pos,(int)((start_pos * CHAR_AVR_WIDTH)+TEXT_X_OFFSET));
        // Allocate a region of this size.

        if (size)
        {
            t_size = 40;
        }
        else
        {
            t_size = 20;
        }

        switch (lang)
        {
        case SCR_LANGUAGE_NORWEGIAN:
            t_lang = NorwegianFont;
            break;
        case SCR_LANGUAGE_SWEDISH:
            t_lang = SwedishFont;
            break;
        case SCR_LANGUAGE_GERMAN:
            t_lang = GermanFont;
            break;
        case SCR_LANGUAGE_ITALIAN:
            t_lang = ItalianFont;
            break;
        case SCR_LANGUAGE_FRENCH:
            t_lang = FrenchFont;
            break;
        case SCR_LANGUAGE_SPANISH:
            t_lang = SpanishFont;
            break;
        case SCR_LANGUAGE_ORIG:
            t_lang = origFont;
            break;
        case SCR_LANGUAGE_ENGLISH:
        default:
            t_lang = EnglishFont;
            break;
        }
        
        int fixed_width = 0;
        int var_width = 0;
        int width_offset = 0;
        for (int k = 0; (unsigned int)k < strlen(&string[start_pos]); k++)
        {
            fixed_width += CHAR_AVR_WIDTH;
        }
        width_offset = (fixed_width - var_width) >> 1;

       
    }
}