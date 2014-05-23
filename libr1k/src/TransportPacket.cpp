#include "TransportPacket.h"
#include <string.h>

namespace libr1k
{
	TransportPacket::TransportPacket( uint8_t *pkt, int packetSze)
	{
		this->SetPacket ( pkt, packetSze );
	}

    void TransportPacket::SetPacketNumber ( const unsigned long pNum )
    {
        this->packetNumber = pNum;
        return;
    }

	void TransportPacket::SetPacket ( uint8_t *data, int packetSze )
	{
		if (data == NULL) return;
		memcpy(this->raw, data, sizeof(uint8_t)*packetSze);
		this->packetSize = packetSze;
		this->findPayload();
	}

	void TransportPacket::findPayload ( void )
	{
		if (raw[3]& 0x10) /* Adaptation field control says there is a payload */
		{
			if (this->AdaptationFieldPresent())
			{
				int adaptation_field_length = this->raw[4];
				Payload = &this->raw[5+adaptation_field_length];
				payloadSize = &this->raw[this->packetSize] - Payload;
			}
			else
			{
				Payload = &this->raw[4];
				payloadSize = PayloadSize;
			}
		}
		else
		{
			payloadSize = 0;
			Payload = NULL;
		}
	}

	uint8_t TransportPacket::GetAdaptationFlags( void ) const
	{
	    return (raw[3] & 0x30) >> 4;
	}

	bool TransportPacket::AdaptationFieldPresent ( void ) const
	{
		return (raw[3] & 0x20) ? true : false;
	}

    bool TransportPacket::PayloadPresent ( void ) const
    {
        return (raw[3] & 0x10) ? true : false;
    }

	void TransportPacket::MakeNULLPID ( void )
	{
		this->raw[0] = 0x47;
		this->raw[1] = 0; // zero the flags before setting the PID
		this->SetPID( NULLPID );
		this->raw[3] = 0x10;

		memset( this->Payload, 0xff, sizeof(uint8_t) * this->packetSize);

		return;
	}

	uint16_t TransportPacket::GetPID() const
	{
		unsigned int temp = 0;
		temp = (raw[1] & 0x1f) << 8;
		return temp | (0xff & raw[2]);
	}

	void TransportPacket::SetPID( const uint16_t PID)
	{
		raw[1] = (raw[1] & 0x70) | (( PID & 0x01f00 ) >> 8 );
		raw[2] = PID & 0x0ff;
	}

	uint8_t TransportPacket::GetCC() const
	{
		return raw[3] & 0x0f;
	}

    void TransportPacket::SetCC(const uint8_t CC)
	{
		raw[3] = (raw[3] & 0xf0) & (CC & 0x0f);
		return;
	}

	uint8_t TransportPacket::GetPUSI() const
	{
		return ((raw[1] & 0x40) ? 1 : 0);
	}

	void TransportPacket::SetPUSI( const bool NewPUSI)
	{
		if (NewPUSI)
		{
			raw[1] = raw[1] | 0x40; // Set PUSI
		}
		else
		{
			raw[1] = raw[1] & 0xbf; // clear PUSI
		}
		return;
	}

	void TransportPacket::SetPayload ( const uint8_t *const newPayload )
	{
		memcpy(this->Payload, newPayload, sizeof(uint8_t) * payloadSize);
		return;
	}

	int TransportPacket::GetPayload ( const uint8_t **payload_ptr )
	{
		*payload_ptr = this->Payload;
		return this->payloadSize;
	}
	
	bool TransportPacket::ConvertTo188 ( void )
	{
		this->packetSize = Ts188Byte;
		return true;
	}

	bool TransportPacket::ConvertTo204 ( void )
	{
		this->packetSize = Ts204Byte;
		memset(&(this->raw[Ts188Byte]), 0xff, (Ts204Byte - Ts188Byte)*sizeof(unsigned char));
		return true;
	}

	int fwritePacket(TransportPacket *const thispacket, FILE * const outputfile)
	{
		int temp = 0;
		if (thispacket == NULL) return 0;
		clearerr(outputfile);
		while (temp < thispacket->packetSize)
		{
			fputc(thispacket->raw[temp], outputfile);
			++temp;
		
			if(ferror(outputfile) != 0)
			{
				return -1;
			}
		}
		return thispacket->packetSize;
	}

	int freadPacket(TransportPacket *const thispacket, FILE *const inputfile)
	{
		int temp = 0;
		if (thispacket == NULL) return 0;
		while( (!feof (inputfile)) && (temp < thispacket->packetSize) )
		{
			thispacket->raw[temp] = (unsigned char) fgetc(inputfile);
			++temp;
		}
		return temp;
	}

	#define SYNC_LENGTH	3
	int fgetTSsync ( FILE *const infile )
	{
		int	chargot = 0;
		int	i = 0, count_204 = 0, count_188 = 0;

		while ( (chargot = fgetc (infile)) != 0x47 );
		while ( (count_188 < SYNC_LENGTH && count_204 < SYNC_LENGTH) && !feof (infile) )
		{
			/* Consume 187 bytes */
			for ( i = 1; i < 188 && !feof (infile); i++ ) chargot = fgetc ( infile );

			chargot = fgetc ( infile );

			if ( chargot == 0x47 )
				count_188++;
			else
			{
				if ( count_188 ) count_188--;

				/* consume upto 204 byte boundary */
				for ( i = 188; i < 203 && !feof (infile); i++ ) chargot = fgetc ( infile );

				chargot = fgetc ( infile );
				if ( chargot == 0x47 )
				{
					count_204++;
					count_188 = 0;
				}
				else
				{
					if ( count_204 ) count_204--;
					while ( (chargot = fgetc (infile)) != 0x47 && !feof (infile) );
				}
			}
		}

		if ( count_204 || count_188 ) 
            return ( count_204 != 0 ) ? 204 : 188;
        return -1;
	}

}

