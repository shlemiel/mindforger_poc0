#!/usr/bin/env bash
#
# MindForger thinking notebook
#
# Copyright (C) 2016-2022 Martin Dvorak <martin.dvorak@mindforger.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

if [ -x "$(command -v tokei)" ]
then
  cd ../.. && tokei
else
  find ../.. -type f -iname \*.cpp -type f -o -iname \*.h | while read F; do cat "$F"; done | wc -l -c
  #find ../.. -type f -iname \*.cpp -type f -o -iname \*.h -exec wc -l -c {} +
fi

# eof
