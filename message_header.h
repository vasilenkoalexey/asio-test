#ifndef H2HEADER_H
#define H2HEADER_H

#include <stdint.h>

namespace network
{
    struct header
    {
        enum
        {
            MAGIC = 0x1234
        };

        enum
        {
            REQUEST_CHANGE_STATE = 0x01,
            ACCEPT_CHANGE_STATE = 0x02
        };

        header( const uint32_t _opcode, const uint32_t _length )
            : magic( 0x1234 )
            , version( 1 )
            , opcode( _opcode )
            , length( _length )
        {}
        header()
            : magic( 0x1234 )
            , version( 1 )
            , opcode( 0 )
            , length( 0 )
        {}
        uint32_t magic;
        uint32_t version;
        uint32_t opcode;
        uint32_t length;
    };

}

#endif