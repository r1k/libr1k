#pragma once

#include "TSPacketHandler.h"

namespace libr1k {

    class TeletextPacketHandler : public TSPacketHandler
    {
    public:
        TeletextPacketHandler(ofstream *str, bool Debug_on = false);
        ~TeletextPacketHandler(void);

        virtual bool DecodeFrame(unsigned char **Frame, unsigned int *FrameSize);
        virtual void PESDecode(PESPacket_t *buf);

        void SetDebugOutput(bool On);

        int FrameCount;

    protected:

        ofstream *outStream;

    private:

        bool DebugOn;
        std::shared_ptr<Log> LogFile;
    };
}