#include "TSPacketHandler.h"
#include "Types.h"
#include <iostream>
#include <algorithm>
using namespace std;

namespace libr1k
{
    bool TSPacketHandler::DecodeFrame(unsigned char **Frame, unsigned int *FrameSize)
    { 
        return false; 
    }

    TSPacketHandler::~TSPacketHandler(void)
    {
        while (!this->PESdata.empty())
        {
            PESPacket_t *NextPacket = this->PESdata.front();
            delete NextPacket;
            this->PESdata.pop_front();
        }
        this->BufferLevel = 0;
    }

    bool TSPacketHandler::FindPESHeaderAndGetPTS(const uint8_t *buf, unsigned long long *PTS)
    {
        uint64_t	pts = 0;

        if ((buf[0] == 0) && (buf[1] == 0) && (buf[2] == 0x01))
        {
            if (stream_id != (uint8_t)buf[3])
            {
                // unexpected stream_id
                return false;
            }

            if (buf[7] & 0x80)
            {
                pts = ((unsigned long long) (buf[9] & 0x0e) >> 1) << 30;	/* 0xx0 then send to most significant 2 bits of
                                                                             * 32 bit unsigned int */
                pts += (buf[10] & 0x0ff) << 22;
                pts += ((buf[11] & 0x0fe) >> 1) << 15;
                pts += (buf[12] & 0x0ff) << 7;
                pts += (buf[13] & 0x0fe) >> 1;
            }
            else
            {
                pts = 0;
            }
            *PTS = pts;
            return true;
        }
        else
        {
            // not the start of a pes packet
            return false;
        }
    }

    bool TSPacketHandler::CheckCCError(unsigned int cc)
    {
        unsigned int expectedNewCC = (this->ContinuityCount + 1) % 0x10;

        if (cc == expectedNewCC)
        {
            return false;
        }
        else
        {
            if (this->doubleCC)
            {
                return true;
            }
            else
            {
                this->doubleCC = true;
                return false;
            }
        }
    }

    void TSPacketHandler::NextPacket(TransportPacket *tsPacket)
    {
        if (!tsPacket->PayloadPresent())
        {
            // no content skip.
            return;
        }

        if (this->CheckCCError(tsPacket->GetCC()))
        {
            cerr << "Continuity Count error found on PID " << tsPacket->GetPID() << endl;
            CurrentState = CCError;
        }

        this->ContinuityCount = tsPacket->GetCC();

        switch (CurrentState)
        {
        case WaitForPesStart:
            if (tsPacket->GetPUSI())
            {
                try
                {
                    PESPacket_t *nextPacket = new PESPacket_t;

                    if (NewPESPacketFound(nextPacket, tsPacket))
                    {
                        PESdata.push_back(nextPacket);

                        if (nextPacket->Started && !nextPacket->Complete)
                        {
                            NextState = WaitForPESData;
                        }
                    }
                }
                catch (...)
                {
                    cerr << "Unable to allocate memory" << endl;
                }
            }
            else
            {
                // No change keep waiting for next packet
                NextState = CurrentState;
            }
            break;
        case WaitForPESData:
        {
            PESPacket_t *lastPacket = PESdata.back();

            if (tsPacket->GetPUSI())
            {
                // error we were waiting for data but we got another payload unit start indicator 
                // signalling that we have the start of another PES packet
                // lets overstamp the previous packet at the back of the queue as it's useless as we don't
                // have the complete packet
                if (lastPacket->payload.size())
                {
                    this->BufferLevel -= lastPacket->payload.size();
                    PESdata.pop_back();
                    lastPacket = nullptr;
                }

                NewPESPacketFound(lastPacket, tsPacket);
            }
            else
            {
                ContinueLastPESPacket(lastPacket, tsPacket);
            }

            if (lastPacket->Started && !lastPacket->Complete)
            {
                NextState = WaitForPESData;
            }
            else
            {
                NextState = WaitForPesStart;
            }
        }
            break;
        case CCError:
            // We have a continuity error so we probably lost some data - throw away the last PES packet
            // if it is incomplete
        default:
            if (!PESdata.empty())
            {
                PESPacket_t *lastPacket = PESdata.back();

                if (!lastPacket->Complete)
                {
                    // The queue is a FIFO so we can't just through the packet away
                    if (lastPacket)
                    {

                        BufferLevel -= lastPacket->payload.size();
                        lastPacket = nullptr;
                        PESdata.pop_back();
                    }
                }
            }
            NextState = WaitForPesStart;
            break;
        }

        CurrentState = NextState;
    }

    unsigned int TSPacketHandler::GetPESPacketSize(const unsigned char * const buf) const
    {
        unsigned int size = (buf[4] << 8) | buf[5]; // pes_packet_length field
        size += 6; //add on size of PES header before payload length
        return size;
    }

    bool TSPacketHandler::NewPESPacketFound(PESPacket_t *pPacket, TransportPacket *tsPacket)
    {
        const uint8_t *pPayload;
        int PayloadSize = tsPacket->GetPayload(&pPayload);
        int PESPacketSize = GetPESPacketSize(pPayload);
        pPacket->Started = false;
        pPacket->Complete = false;

        if (pPayload != NULL)
        {
            if (FindPESHeaderAndGetPTS(pPayload, &(pPacket->PTS)))
            {
                pPacket->type = stream_id;
                pPacket->SetDataLength(PESPacketSize);

                int copy = min(PayloadSize, pPacket->pesPacketLength - pPacket->GetCurrentDataLength());
                pPacket->AddNewData(pPayload, copy);

                BufferLevel += PayloadSize;

                return true;
            }
        }
        return false;
    }



    bool TSPacketHandler::ContinueLastPESPacket(PESPacket_t *pPacket, TransportPacket *tsPacket)
    {
        const uint8_t *pPayload = NULL;
        const int PayloadSize = tsPacket->GetPayload(&pPayload);

        if (pPayload != NULL)
        {
            const int copy = min(PayloadSize, pPacket->pesPacketLength - pPacket->GetCurrentDataLength());
            pPacket->AddNewData(pPayload, copy);

            BufferLevel += copy;

            if (pPacket->GetCurrentDataLength() < pPacket->pesPacketLength)
            {
                pPacket->Complete = false;
            }
            else if (pPacket->GetCurrentDataLength() == pPacket->pesPacketLength)
            {
                pPacket->Complete = true;
            }
            return true;
        }

        return false;
    }

    void TSPacketHandler::PCRTick(unsigned long long PCR)
    {
        if (!PESdata.empty())
        {
            PESPacket_t *NextPacket = PESdata.front();

            if (NextPacket)
            {
                if (!NextPacket->Started && !NextPacket->Complete)
                {
                    // This packet was placed in the buffer but later discarded
                    // probably because we didn't get all the data to complete it.
                    this->PESdata.pop_front();
                }
                else if (NextPacket->Complete)
                {
                    enum { PlayPacket, DiscardPacket, Wait } PacketControl = Wait;
#ifdef WIN32
                    unsigned __int64 diff = -1;
#else
                    uint64_t diff = -1;
#endif

                    if (PCR == 0)
                    {
                        PacketControl = PlayPacket;
                    }
                    else if (NextPacket->PTS < PCR)
                    {
                        // Wrap around or packet is stale
                        if ((NextPacket->PTS > 0xFFFFFFFF) && (PCR < 0x00000fff))
                        {
                            // Wrap around
                            diff = PCR + (0x1ffffffff - NextPacket->PTS);
                        }
                        else
                        {
                            PacketControl = DiscardPacket;
                        }
                    }
                    else
                    {
                        diff = NextPacket->PTS - PCR;
                    }

                    if (PacketControl != DiscardPacket)
                    {
                        if (diff < 200)
                        {
                            PacketControl = PlayPacket;
                        }
                    }

                    if (PacketControl != Wait)
                    {
                        if (PacketControl == DiscardPacket)
                        {
                            //cerr << "Stale packet found" << endl;				
                        }
                        else if (PacketControl == PlayPacket)
                        {
                            // packet is due in the next ~21ms
                            PESDecode(NextPacket);
                        }

                        BufferLevel -= NextPacket->GetCurrentDataLength();
                        delete NextPacket;
                        // data deleted now pop the packet from th queue
                        PESdata.pop_front();
                    }
                }
                // else wait until frame is ready
            }
            else
            {
                this->PESdata.pop_front();
            }
        }
        //empty queue nothing to do.
    }


    void TSPacketHandler::PESDecode(PESPacket_t *buf)
    {
        // Read PES header
        uint8_t *PES_data =buf->GetPESData(); // The start of the data is the number of bytes in the PES header length field
        // added to the first byte after the PES header length field
        // Need to adjust PESPacketSize to make it just the payload size
        unsigned int PESPacketSize = buf->GetPESDataLength();

        DecodeFrame(&PES_data, &PESPacketSize);
    }


}