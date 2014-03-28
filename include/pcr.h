#pragma once

#include <iostream>
#include "libr1k.h"
#include "TransportPacket.h"
#include "Types.h"

using namespace std;
namespace libr1k
{
    class pcr
	{
	private:
		uint64_t	PCRHistory[3];
		uint64_t	PCRPacketNumber[3];
		uint64_t	PCRPerPacket;
		bool		ready ( void )	{ return PCRPerPacket != 0; }
		void		CalculateAverage ( void );

	public:
		uint64_t getPCR ( uint64_t packetNumber );
		pcr (void);
		void print ( uint64_t packetNum, ostream &output );
		
		void newValue(uint64_t newPCR, uint64_t packetNumber);
		void NextPacket ( const TransportPacket *tsPacket );
	};

    class StreamTime // Singleton
	{
	public:
		static StreamTime& getInstance();
		~StreamTime() {}

		void     StartTime_Set(uint64_t SCR);
		uint64_t StartTime()
		{ 
			uint64_t rv;
			StreamStartTimeSet_val ? rv = StreamStartTime_val : rv = 0; 
			return rv;
		}
		
		uint64_t TimeElapsed(uint64_t now) 
		{ 
			uint64_t rv; 
			StreamStartTimeSet_val ? rv = now - StreamStartTime_val : rv = now;
			return rv;
		}

		// Bunch of static functions for processing time.

		static uint64_t time_33_in_ms(uint64_t time);
		static uint64_t time_32_in_ms(uint64_t time);

		static void milliseconds_to_hms_ms(const uint64_t time, unsigned int &h, unsigned int &m, unsigned int &s, unsigned int &ms);

		static ostream& print_time_hms_ms(ostream& os, uint64_t time_in_ms);
		static ostream& print_time_hms_ms(ostream& os, const unsigned int h, const unsigned int m, const unsigned int s, const unsigned int ms);
		static unsigned int pts_ticks33_per_ms;
		static unsigned int pts_ticks32_per_ms;
		
	private:

		StreamTime() : StreamStartTime_val(0), StreamStartTimeSet_val(false) {}
		uint64_t StreamStartTime_val;
		bool     StreamStartTimeSet_val;

		StreamTime(const StreamTime&) = delete;
		void operator=(StreamTime const &) = delete;
	};
}
