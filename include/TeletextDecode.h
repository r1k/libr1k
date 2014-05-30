#pragma once

#include "esPacketDecoder.h"
#include "TSPacketHandler.h"
#include <memory>
#include <cstdarg>

namespace libr1k {

#define ILLEGAL_TTX_SUB_PAGE    999
#define ALL_PAGES 1000

#define END_BOX                 0x0A    // End of string
#define START_BOX               0x0B    // Start of actual subtitle string
#define NORMAL_HEIGHT           0x0C    // Normal height ( assumed at start of line )
#define DOUBLE_HEIGHT           0x0D    // Double height ( always on for subtitles  )
#define BLACK_BG                0x1C    // Set background black ( assumed at start of line )
#define NEW_BG                  0x1D    // Set current fg colour to bg colour
#define CTL_C4                  0x08    // Clear subtitles

#define REGION_HEIGHT         (20)      // Region height
#define X_STR_OFFSET          (2)       // Horisontal position of string in region
#define Y_STR_OFFSET          (1)       // Vertical position of string in region
#define Y_STR_OFFSET_DBL      (2)       // Vertical position of string in region ( double height font )
#define H_REG_HEIGHT          (20)      // Min. vertical height of region
#define H_REG_HEIGHT_DBL      (40)      // Min. vertical height of region ( double height font )
#define TEXT_X_OFFSET         (40 - 16) 
    // Where to start a string when no spacing in front
    // -1 char, since START_BOX;START_BOX begins each line
    // i.e. subtitling can never use offsets 0 nor 1
#define CHAR_AVR_WIDTH        ((720 - TEXT_X_OFFSET)/42)  // assuming 2 chars off screen edge not visible
    // Used to calculate string start position

    enum
    {
        // Used to select charsets when language is not defined in teletext spec.
        NO_SPECIAL_LANG = 0,
        SPECIAL_LANG_NOR,
        SPECIAL_LANG_DAN,
        ILLEGAL_SPECIAL_LANG
    };
    typedef enum{
        SCR_LANGUAGE_DEFAULT,
        SCR_LANGUAGE_NORWEGIAN,
        SCR_LANGUAGE_ENGLISH,
        SCR_LANGUAGE_SWEDISH,
        SCR_LANGUAGE_GERMAN,
        SCR_LANGUAGE_ITALIAN,
        SCR_LANGUAGE_FRENCH,
        SCR_LANGUAGE_SPANISH,
        SCR_LANGUAGE_EXTRA,
        SCR_LANGUAGE_ORIG,
    } SCR_TXT_FontLanguage_t;

    typedef enum FontLanguage_e {
        EnglishFont,
        FrenchFont,
        GermanFont,
        SwedishFont,
        SpanishFont,
        ItalianFont,
        NorwegianFont,
        extraCharsFont,
        origFont
    } FontLanguage_t;


    /* Special value for color meaning don't change from current value */
#define UNCHANGED_COLOR       0xFFFFFFFF
#define SCR_MAX_MIX_WEIGHT    (63)
    enum {
        BLACK,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        PURPLE,
        CYAN,
        WHITE,
        GRAY,
        DRED,
        DGREEN,
        DYELLOW,
        DBLUE,
        DPURPLE,
        DCYAN,
        DWHITE,
        PALETTE_SIZE,

        // Teletext only has 8 colours defined (0 to 7); 
        // these match SCR's colour definitions given above (BLACK to WHITE)
        TTXSUBT_TRANS_BACKGROUND = 8,
        TTXSUBT_WHITE_FOREGROUND = 9
    };


#define STSUBT_MODE_CHANGE     1
#define STSUBT_NORMAL_CASE     2

    typedef struct PCS_s {
        uint16_t         page_id;
        uint8_t          acquisition_mode;
        uint8_t          page_version_number;
    } PCS_t;

    typedef struct
    {
        unsigned int address;
        unsigned int mode;
        unsigned int data;
        unsigned int character_to_update;
        unsigned int symbol_offset;
        unsigned int standard_char;
        unsigned int line_num;
    } decoded_triplet;

    class SubtitleLine
    {
        // Class used for storing one line of subtitles
        friend class TtxSubtitlePage;           // Needs access to this class attributes
    private:
        char *str;                          // Actual text string for this line
        uint8_t lineNo;                        // Line number
        uint8_t dblSize;                       // Double size
        uint8_t fg_color;                      // Foreground color
        uint8_t bg_color;                      // backround color
        uint8_t box;                           // box type
        int CalculateVertPos(void);

    public:
        SubtitleLine(char *string, uint8_t start_pos, uint8_t line, uint8_t bg_color, uint8_t fg_color,
            uint8_t box, uint8_t size, SCR_TXT_FontLanguage_t lang = SCR_LANGUAGE_DEFAULT,
            int ECCount = 0, decoded_triplet *dTrip = NULL);

        ~SubtitleLine();                    // Destructor
        bool operator==(const SubtitleLine& toCompare);
    };

    class TtxSubtitlingTask
    {
    public:
        std::string *currentString;

        TtxSubtitlingTask(unsigned int page)
        {    
            subtPage = page;

            // Get also magasine
            uint8_t ttx_sub_mag;
            if (subtPage != ILLEGAL_TTX_SUB_PAGE)
            {
                ttx_sub_mag = (subtPage / 100) & 0x07;
            }
            else
            {
                ttx_sub_mag = 0xFF;
            }
        }

        virtual ~TtxSubtitlingTask() {}

        void ParseTtxData(const uint8_t *const tdata);
        void Decode_data_block(const uint8_t *data, string &string_represenation);
        uint8_t Check_subtitling_field(const uint8_t *const subtitling_field);
        void filter_text(const uint8_t *const ttx_local);

        void SetPage(const int page){ subtPage = page; }
        void SetColor(uint8_t fg, uint8_t bg){ fgClr = fg; bgClr = bg; }

        bool print_ttx(const uint8_t *b, uint8_t *bff, uint8_t *start_pos, uint8_t *b_col, uint8_t *f_col,
            uint8_t *bg_colours, uint8_t *fg_colours);

    private:
        uint16_t _GetSubtPage(const uint8_t *data, const uint8_t magazine, const uint8_t row);
        void TtxtGetPage(const uint8_t *data, uint8_t &magazine, uint8_t &row) const;

        uint16_t subtPage;
        uint8_t ttx_sub_mag;
        SCR_TXT_FontLanguage_t lang;
        uint8_t bgClr;
        uint8_t fgClr;
    };

    class TeletextPacketHandler : public TSPacketHandler
    {
    public:
        TeletextPacketHandler(ofstream *str,  ofstream *txtstream, unsigned int page, bool Debug_on = false);
        ~TeletextPacketHandler(void);

        virtual int DecodeFrame();
        virtual void PESDecode(PESPacket_t *buf);

        void SetDebugOutput(bool On);

        int FrameCount;

    protected:

        ofstream &outStream;
        ofstream &decStream;

        void LogMessage(const int errorLevel, const char *message, ...)
        {
            if (LogFile != nullptr)
            {
                static const int bufLength = 2000;
                char formatted_string[bufLength];

                va_list args;
                va_start(args, message);
                vsnprintf_s(formatted_string, bufLength, message, args);
                LogFile->AddMessage(errorLevel, formatted_string);
                va_end(args);
            }
        }

        void LogMessage(const int errorLevel, string message)
        {
            if (LogFile != nullptr)
            {
                LogFile->AddMessage(errorLevel, message);
            }
        }

        void writeOutputStreamHeader()
        {
            outStream << "Teletext Decoder" << endl;
            outStream << "PTS,data_identifier,data_unit_id,data_unit_length,field_parity,line_offset,";
            outStream << "framing_code,magazine_and_packet_address, data" << endl;
        
            decStream << "Teletext Decoder" << endl;
            decStream << "field_parity, line_offset,";
            decStream << "framing_code,magazine_and_packet_address, data" << endl;
        }
    private:

        bool DebugOn;
        std::shared_ptr<Log> LogFile;
        DataBuffer_u8 buffer;

        TtxSubtitlingTask processor;
        int page;
    };
}