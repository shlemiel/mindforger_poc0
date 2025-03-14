/*
 note_char_provider.cpp     MindForger thinking notebook

 Copyright (C) 2016-2022 Martin Dvorak <martin.dvorak@mindforger.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "note_char_provider.h"

namespace m8r {

NoteCharProvider::NoteCharProvider(Note* note, char delimiter)
    : CharProvider{}
{
    // N description (split in lines) streaming was complicated (check) and therefe slow - narrowing is faster
    s.assign(note->getName());
    s += delimiter;
    s += note->getDescriptionAsString();

    p = new StringCharProvider{s};
}

NoteCharProvider::~NoteCharProvider()
{
    delete p;
}

} // m8r namespace
