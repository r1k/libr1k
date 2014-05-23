#include "TeletextDecode.h"

using namespace std;
namespace libr1k{
    
    TeletextPacketHandler::TeletextPacketHandler(ofstream *str, bool Debug_on)
    {
        outStream = str;
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

}