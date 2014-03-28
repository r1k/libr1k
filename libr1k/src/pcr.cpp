#include "pcr.h"
#include "TransportPacket.h"
#include <iomanip>

namespace libr1k
{
	StreamTime& StreamTime::getInstance()
	{
		static StreamTime inst;

		return inst;
	}

	void StreamTime::StartTime_Set(uint64_t SCR)
	{
		if (StreamStartTime_val)
			return;

		StreamStartTimeSet_val = true;
		StreamStartTime_val = SCR;
	}

	unsigned int StreamTime::pts_ticks33_per_ms = 90;
	unsigned int StreamTime::pts_ticks32_per_ms = 45;

	uint64_t StreamTime::time_33_in_ms(uint64_t time)
	{
		return time / pts_ticks33_per_ms;
	}
	
	uint64_t StreamTime::time_32_in_ms(uint64_t time)
	{
		return time / pts_ticks32_per_ms;
	}

	void StreamTime::milliseconds_to_hms_ms(const uint64_t time, unsigned int &h, unsigned int &m, unsigned int &s, unsigned int &ms)
	{
		const unsigned int milliseconds_per_second = 1000;
		const unsigned int milliseconds_per_minute = 60 * milliseconds_per_second;
		const unsigned int milliseconds_per_hour   = 60 * milliseconds_per_minute;

		h = m = s = ms = 0;
		uint64_t time_remaining = time;

		h = static_cast<unsigned int>(time_remaining / milliseconds_per_hour); // unlikely to need a long long for the num of hours
		time_remaining -= h * milliseconds_per_hour;

		m = static_cast<unsigned int>(time_remaining / milliseconds_per_minute);  // can't be long long after above divide;
		time_remaining -= m * milliseconds_per_minute;

		s = static_cast<unsigned int>(time_remaining / milliseconds_per_second);  // can't be long long after above divide;
		ms = static_cast<unsigned int>(time_remaining - s * milliseconds_per_second);  // can't be long long after above divide;

	}

	ostream& StreamTime::print_time_hms_ms(ostream& os, uint64_t time_in_ms)
	{
		unsigned int hours, minutes, seconds, milliseconds;
		StreamTime::milliseconds_to_hms_ms(time_in_ms, hours, minutes, seconds, milliseconds);
		
		return StreamTime::print_time_hms_ms(os, hours, minutes, seconds, milliseconds);	
	}

	ostream& StreamTime::print_time_hms_ms(ostream& os, const unsigned int h, const unsigned int m, const unsigned int s, const unsigned int ms)
	{
		auto fill_chr = os.fill();
		auto width = os.width();
		os.fill('0');
		os << setw(2) << h << ":" << setw(2) << m << ":" << setw(2) << s << "." << setw(3) << ms;
		os.fill(fill_chr);
		os.width(width);
		return os;
	}

	pcr::pcr ( void ) :
			PCRPerPacket( 0 )
	{
		PCRHistory[0] = 0;
		PCRHistory[1] = 0;
		PCRHistory[2] = 0;
		PCRPacketNumber[0] = 0;
		PCRPacketNumber[1] = 0;
		PCRPacketNumber[2] = 0;
	}

	uint64_t pcr::getPCR(uint64_t packetNumber)
	{
		if ( ready () )
		{
			return ( (PCRHistory[2] + (PCRPerPacket * (packetNumber - PCRPacketNumber[2]))) & 0x1ffffffff );
		}
		else
		{
			return 0;
		}
	}

	void pcr::CalculateAverage ( void )
	{
		PCRPerPacket = ( PCRHistory[2] - PCRHistory[0] ) / ( PCRPacketNumber[2] - PCRPacketNumber[0] );
	}

	void pcr::NextPacket ( const TransportPacket *tsPacket)
	{
		const uint8_t *pPkt = tsPacket->raw;
		// check for transport stream packet adaptation field which is where the pcr
		// should be if present
		if ( pPkt[3] & 0x20 )				// it contains an adaptation field
		{
			int adaptation_field_length = pPkt[4];
			if ( adaptation_field_length >= 7 )
			{
				if ( pPkt[5] & 0x10 )
				{	// if PCR flag
					uint64_t	newPCR = pPkt[6];
					newPCR = ( newPCR << 8 ) | pPkt[7];
					newPCR = ( newPCR << 8 ) | pPkt[8];
					newPCR = ( newPCR << 8 ) | pPkt[9];
					newPCR = ( newPCR << 1 ) | ( (pPkt[10] | 0x80) >> 7 );
		
					newValue ( newPCR, tsPacket->packetNumber );
				}
			}
		}
	}

	void pcr::newValue(uint64_t newPCR, uint64_t packetNumber)
	{
		PCRHistory[0] = PCRHistory[1];
		PCRPacketNumber[0] = PCRPacketNumber[1];

		PCRHistory[1] = PCRHistory[2];
		PCRPacketNumber[1] = PCRPacketNumber[2];

		PCRHistory[2] = newPCR;
		PCRPacketNumber[2] = packetNumber;

		if ( PCRPacketNumber[0] != 0 )
		{
			CalculateAverage ();
		}

		StreamTime::getInstance().StartTime_Set(newPCR);
	}

	void pcr::print(uint64_t packetNum, ostream &output)
	{
		uint64_t	tempPCR = getPCR(packetNum);
		output << dec << tempPCR << ",";
	}
}