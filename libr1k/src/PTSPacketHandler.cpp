#include "PTSPacketHandler.h"
#include "BitStreamReader.h"

#include "pcr.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string.h>

using namespace std;
namespace libr1k
{
	PTSPacketHandler::PTSPacketHandler(ofstream **str, pcr *p, bool Debug_on)
	{
		this->outStream = *str;
		this->stream_id = -1;
		this->filePerPes = false;
        this->PCR = p;
		
		this->DebugOn = Debug_on;
		if (this->DebugOn)
		{
			this->LogFile = new Log();
		}
		else
		{
			this->LogFile = NULL;
		}

		*this->outStream << "packet number,pcr,pts,pespacketlength,bytesleft,comment";
	}

	PTSPacketHandler::~PTSPacketHandler(void)
	{
		if (this->OutputFile)
			delete this->OutputFile;
		if (this->LogFile)
			delete this->LogFile;
	}

	void PTSPacketHandler::SetDebugOutput(bool On)
	{
		if (this->DebugOn != On)
		{
			if (On)
			{
				// Was off now need to turn it on
				if (this->LogFile != NULL)
				{
					// this should have been null because we are not logging, so we probably should delete this
					delete this->LogFile;
				}
				this->LogFile = new Log();
				this->DebugOn = On;
			}
			else
			{
				// Was on now need to turn it off
				if (this->LogFile != NULL)
				{
					// this should have been null because we are not logging, so we probably should delete this
					delete this->LogFile;
				}
				this->DebugOn = On;
			}
		}
	}

	void PTSPacketHandler::PESDecode ( PESPacket_t *const buf )
	{
		// Read PES header
		// Here we have a full PES packet just dump it out.
	    return;
	}

	bool PTSPacketHandler::NewPESPacketFound(PESPacket_t *pPacket, TransportPacket *tsPacket)
    {
        const uint8_t *pPayload;
        int PayloadSize = tsPacket->GetPayload(&pPayload);
        pPacket->Started = false;
        pPacket->Complete = false;
        pPacket->bytesStored = 0;

        if (pPayload != NULL)
        {
            if ( this->FindPESHeaderAndGetPTS(pPayload, &(pPacket->PTS)) )
            {

                int PESPacketSize = this->GetPESPacketSize(pPayload);

                *this->outStream << pPacket->PTS << ",";
                pPacket->type = this->stream_id;

                pPacket->pesPacketLength = PESPacketSize;
                *this->outStream << PESPacketSize << ",";
                pPacket->Started = true;

                *this->outStream << PESPacketSize - PayloadSize << ",";

                if (PayloadSize < PESPacketSize)
                {
                    pPacket->Complete = false;
                }
                else
                {
                    pPacket->Complete = true;
                }

                pPacket->bytesStored += PayloadSize;

                return true;
            }
        }
        return false;
    }

	bool PTSPacketHandler::ContinueLastPESPacket(PESPacket_t *pPacket, TransportPacket *tsPacket)
    {
        const uint8_t *pPayload = NULL;
        int PayloadSize = tsPacket->GetPayload(&pPayload);

        if (pPayload != NULL)
        {
            *this->outStream << pPacket->PTS << ",";
            *this->outStream << pPacket->pesPacketLength << ",";
            *this->outStream << pPacket->pesPacketLength - PayloadSize << ",";

            pPacket->bytesStored += PayloadSize;
            if (pPacket->bytesStored < pPacket->pesPacketLength)
            {
                pPacket->Complete = false;
            }
            else
            {
                pPacket->Complete = true;
            }
            return true;
        }
        return false;
    }

    void PTSPacketHandler::NextPacket( TransportPacket *tsPacket )
    {
        *this->outStream << tsPacket->packetNumber << "," << this->PCR->getPCR(tsPacket->packetNumber) << ",";
        if (!tsPacket->PayloadPresent())
        {
            *this->outStream << ",,,Adaptation flags" << tsPacket->GetAdaptationFlags() << endl;
            return;
        }

        if (this->CheckCCError(tsPacket->GetCC()))
        {
            *this->outStream << "CCError";
            CurrentState = CCError;
        }

        this->ContinuityCount = tsPacket->GetCC();

        switch(CurrentState)
        {
            case WaitForPesStart:
                if (tsPacket->GetPUSI())
                {
                    PESPacket_t *nextPacket = new PESPacket_t;

                    if (nextPacket)
                    {
                        if (NewPESPacketFound(nextPacket, tsPacket))
                        {
                            PESdata.push_back(nextPacket);

                            if (nextPacket->Started && !nextPacket->Complete)
                            {
                                NextState = WaitForPESData;
                            }
                        }
                    }
                    else
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
                    PESPacket_t *lastPacket = this->PESdata.back();

                    if (tsPacket->GetPUSI())
                    {
                        // error we were waiting for data but we got another payload unit start indicator
                        // signalling that we have the start of another PES packet
                        // lets overstamp the previous packet at the back of the queue as it's useless as we don't
                        // have the complete packet
                        if (lastPacket->payload)
                        {
                            this->BufferLevel -= (lastPacket->nextFreeByte - lastPacket->payload);
                            delete lastPacket;
                            lastPacket->payload = NULL;
                            lastPacket->nextFreeByte = NULL;
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
                break;
            default:
                if (!this->PESdata.empty())
                {
                    *this->outStream << "Some error, we shouldn't be here - except maybe on start up";
                    PESPacket_t *lastPacket = this->PESdata.back();
                    if (!lastPacket->Complete)
                    {
                        // The queue is a FIFO so we can't just through the packet away
                        if (lastPacket)
                        {
                            lastPacket->Started = false;
                            lastPacket->Complete = false;
                            this->BufferLevel -= (lastPacket->nextFreeByte - lastPacket->payload);
                            delete lastPacket;
                        }
                    }
                }
                NextState = WaitForPesStart;
                break;
        }
        *this->outStream << ",Adaptation flags" << tsPacket->GetAdaptationFlags() << endl;

        CurrentState = NextState;
    }

}
