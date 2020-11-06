#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>

uint32_t uint32BE(uint8_t* string) {
    return ((string[0] & 0xff) << 24 | (string[1] & 0xff) << 16 | (string[2] & 0xff) << 8 | string[3] & 0xff) << 0;
}

int parser( char * filename )
{
    uint32_t length = 0;
    uint64_t dataLength = 0;
    char type[ 5 ];
    char * data;
    data = NULL;
    memset( type, 0, 5 );
    std::ifstream stream;
    int track_count = 0;
    int mp4a_track_number = 0;
    bool mp4a_found = false;
    uint32_t stsc_number_of_entries;
    uint32_t stsz_sample_size;
    uint32_t stco_number_of_entries;
    uint32_t stco_chunk_offset;
    std::vector<int> stsc_table;
    std::vector<int> stco_table;

    stream.open( filename, std::ios::binary | std::ios::in );

    if( stream.is_open() == false || stream.good() == false )
        std::cout << "Couldn't open media file" << std::endl;

    while( !stream.eof() )
    {
        uint8_t c[ 4 ];

        // Read atom length
        stream.read( ( char * )c, 4 );
        length = uint32BE((uint8_t*)c);
        dataLength = length - 8;
        // Read atom name
        stream.read( ( char * )type, 4 );

        if( strcmp( type, "trak" ) == 0 )
        {
            track_count++;
            continue;
        }

        if( strcmp( type, "moov" ) == 0 ||
                strcmp( type, "mdia" ) == 0 ||
                strcmp( type, "minf" ) == 0 ||
                strcmp( type, "stbl" ) == 0 ||
                strcmp( type, "stsd" ) == 0
        )
        {
            continue;
        }

        if( strcmp( type, "mp4a" ) == 0 && !mp4a_found )
        {
            mp4a_found = true;
            mp4a_track_number = track_count;
            stream.ignore( dataLength );

            // We found the track we need. Now continue to parse the rest of sample table atoms.
            // STTS atom
            stream.read( ( char * )c, 4 );
            dataLength = uint32BE((uint8_t*)c) - 8;
            stream.read( ( char * )type, 4 );

            if( strcmp( type, "stts" ) == 0 )
            {
                    stream.ignore( dataLength );
            }
           // STSC atom
            stream.read( ( char * )c, 4 );
            dataLength = uint32BE((uint8_t*)c) - 8;
            stream.read( ( char * )type, 4 );

            if( strcmp( type, "stsc" ) == 0 )
            {
                    stream.ignore( dataLength );

                    // Ignore version and flags fields
                    stream.ignore( 4 );

                    // Read number of entries
                    uint8_t stsc_nr[ 4 ];
                    stream.read( (char *) stsc_nr, 4 );
                    stsc_number_of_entries = uint32BE((uint8_t*)stsc_nr);

                    // Read sample-to-chunk table 

                    int i = 0;

                    // Read first_chunk field
                    uint8_t stsc_first_chunk[ 4 ];
                    stream.read( (char *) stsc_first_chunk, 4 );
                    uint32_t first_chunk = uint32BE((uint8_t*)stsc_first_chunk);

                    // Read sample per chunk field
                    uint8_t stsc_samp_per_chunk[ 4 ];
                    stream.read( (char *) stsc_samp_per_chunk, 4 );
                    uint32_t stsc_sam_per_chunk = uint32BE((uint8_t*)stsc_samp_per_chunk);

                    // Ignore description 
                    stream.ignore( 4 );

                    while( i < stsc_number_of_entries - 1 )
                    {
                            // Read first_chunk field
                            uint8_t stsc_new_first_chunk[ 4 ];
                            stream.read( (char *) stsc_new_first_chunk, 4 );
                            uint32_t new_first_chunk = uint32BE((uint8_t*)stsc_new_first_chunk);

                            // Read sample per chunk field
                            uint8_t stsc_new_samp_per_chunk[ 4 ];
                            stream.read( (char *) stsc_new_samp_per_chunk, 4 );
                            uint32_t stsc_new_sam_per_chunk = uint32BE((uint8_t*)stsc_new_samp_per_chunk);

                            // Ignore description 
                            stream.ignore( 4 );

                            for (int j = 0; j < ( new_first_chunk - first_chunk ) ; j++)
                            {
                                    stsc_table.push_back( stsc_sam_per_chunk );
                            }

                            first_chunk = new_first_chunk;
                            stsc_sam_per_chunk = stsc_new_sam_per_chunk;

                            i++;
                    }
                    stsc_table.push_back( stsc_sam_per_chunk );
            }
            // STSZ atom
            stream.read( ( char * )c, 4 );
            dataLength = uint32BE((uint8_t*)c) - 8;
            stream.read( ( char * )type, 4 );

            if( strcmp( type, "stsz" ) == 0 )
            {
                    // Ignore version and flags fields
                    stream.ignore( 4 );

                    // Read sample_size field
                    uint8_t stsz[ 4 ];
                    stream.read( (char *) stsz, 4 );
                    stsz_sample_size = uint32BE((uint8_t*)stsz);

                    // Ignore number of entries and table 
                    stream.ignore( dataLength - 8 );
            }

            // STCO atom
            stream.read( ( char * )c, 4 );
            dataLength = uint32BE((uint8_t*)c) - 8;
            stream.read( ( char * )type, 4 );

            std::cout << "type = " << type << std::endl;
            if( strcmp( type, "stco" ) == 0 )
            {
                    std::cout << "type = " << type << std::endl;
                    // Ignore version and flags fields
                    stream.ignore( 4 );

                    // Read number of entries
                    uint8_t stco[ 4 ];
                    stream.read( (char *) stco, 4 );
                    stco_number_of_entries = uint32BE((uint8_t*)stco);

                    int i = 0;

                    while( i < stco_number_of_entries - 1 )
                    {
                            // Read chunk offset table 
                            uint8_t stco_offset[ 4 ];
                            stream.read( (char *) stco_offset, 4 );
                            stco_chunk_offset = uint32BE((uint8_t*)stco_offset);

                            stco_table.push_back( stco_chunk_offset );
                    }
            }

            continue;
        }
       if( strcmp( type, "stco" ) == 0 )
        {
            stream.ignore( dataLength );
            continue;
        }

        if( strcmp( type, "ftyp" ) == 0 ||
                strcmp( type, "mvhd" ) == 0 ||
                strcmp( type, "avc1" ) == 0 ||
                strcmp( type, "ac-3" ) == 0 ||
                strcmp( type, "iods" ) == 0 ||
                strcmp( type, "mdat" ) == 0 ||
                strcmp( type, "tkhd" ) == 0 ||
                strcmp( type, "edts" ) == 0 ||
                strcmp( type, "mdhd" ) == 0 ||
                strcmp( type, "hdlr" ) == 0 ||
                strcmp( type, "hglr" ) == 0 ||
                strcmp( type, "vmhd" ) == 0 ||
                strcmp( type, "dinf" ) == 0 ||
                strcmp( type, "smhd" ) == 0 ||
                strcmp( type, "dref" ) == 0 ||
                strcmp( type, "stts" ) == 0 ||
                strcmp( type, "ctts" ) == 0 ||
                strcmp( type, "gmhd" ) == 0 ||
                strcmp( type, "stss" ) == 0 ||
                strcmp( type, "stsc" ) == 0 ||
                strcmp( type, "stsz" ) == 0 ||
                strcmp( type, "tmcd" ) == 0 ||
                strcmp( type, "udta" ) == 0 ||
                strcmp( type, "cmov" ) == 0 ||
                strcmp( type, "cmvd" ) == 0 ||
                strcmp( type, "c064" ) == 0 ||
                strcmp( type, "dcom" ) == 0 ||
                strcmp( type, "elst" ) == 0 ||
                strcmp( type, "fiel" ) == 0 ||
                strcmp( type, "ipro" ) == 0 ||
                strcmp( type, "junk" ) == 0 ||
                strcmp( type, "meta" ) == 0 ||
                strcmp( type, "mfra" ) == 0 ||
                strcmp( type, "moof" ) == 0 ||
                strcmp( type, "mvex" ) == 0 ||
                strcmp( type, "pict" ) == 0 ||
                strcmp( type, "pnot" ) == 0 ||
                strcmp( type, "rdrf" ) == 0 ||
                strcmp( type, "rmcd" ) == 0 ||
                strcmp( type, "rmcs" ) == 0 ||
                strcmp( type, "rmda" ) == 0 ||
                strcmp( type, "rmdr" ) == 0 ||
                strcmp( type, "rmqu" ) == 0 ||
                strcmp( type, "rmra" ) == 0 ||
                strcmp( type, "rmvc" ) == 0 ||
                strcmp( type, "sinf" ) == 0 ||
                strcmp( type, "skip" ) == 0 ||
                strcmp( type, "traf" ) == 0 ||
                strcmp( type, "uuid" ) == 0 ||
                strcmp( type, "wide" ) == 0 ||
                strcmp( type, "wfex" ) == 0 ||
                strcmp( type, "free" ) == 0
        )
        {
            stream.ignore( dataLength );

            continue;
        }
    }

    if( !mp4a_found )
            std::cout << "No MP3 audio track in the media file" << std::endl;

    if( stream.is_open() )
        stream.close();

        return 0;
}

int main( int argc, char * argv[] )
{
    if( argc != 2 )
    {
        std::cout << "Please provide a media file" << std::endl;
    }

    parser( argv[ 1 ] );
    return 0;
}
