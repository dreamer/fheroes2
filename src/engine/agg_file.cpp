/***************************************************************************
 *   Free Heroes of Might and Magic II: https://github.com/ihhub/fheroes2  *
 *   Copyright (C) 2020                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <string>

#include "agg_file.h"

namespace fheroes2
{
    AGGFile::AGGFile() {}

    bool AGGFile::isGood() const
    {
        return !_stream.fail() && _files.size();
    }

    bool AGGFile::open( const std::string & fileName )
    {
        if ( !_stream.open( fileName, "rb" ) )
            return false;

        const size_t size = _stream.size();
        const size_t count = _stream.getLE16();
        const size_t fileRecordSize = sizeof( uint32_t ) * 3;

        if ( count * ( fileRecordSize + _maxFilenameSize ) >= size )
            return false;

        StreamBuf fileEntries = _stream.toStreamBuf( count * fileRecordSize );
        const size_t nameEntriesSize = _maxFilenameSize * count;
        _stream.seek( size - nameEntriesSize );
        StreamBuf nameEntries = _stream.toStreamBuf( nameEntriesSize );

        for ( size_t i = 0; i < count; ++i ) {
            const std::string & name = nameEntries.toString( _maxFilenameSize );
            fileEntries.getLE32(); // skip CRC (?) part
            const uint32_t fileOffset = fileEntries.getLE32();
            const uint32_t fileSize = fileEntries.getLE32();
            _files[name] = std::make_pair( fileSize, fileOffset );
        }
        if ( _files.size() != count ) {
            _files.clear();
            return false;
        }
        return !_stream.fail();
    }

    const std::vector<uint8_t> & AGGFile::read( const std::string & fileName )
    {
        if ( _key != fileName ) {
            auto it = _files.find( fileName );
            if ( it != _files.end() ) {
                _key = fileName;
                const auto & fileParams = it->second;
                if ( fileParams.first > 0 ) {
                    _stream.seek( fileParams.second );
                    _body = _stream.getRaw( fileParams.first );
                }
                else {
                    _body.clear();
                }
            }
            else {
                _key.clear();
                _body.clear();
            }
        }
        return _body;
    }
}
