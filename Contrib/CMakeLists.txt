# *********************************************************************************** #
#                                                                                     #
#  Copyright (c) 2018 by Bodo Bauer <bb@bb-zone.com>                                  #
#                                                                                     #
#  This program is free software: you can redistribute it and/or modify               #
#  it under the terms of the GNU General Public License as published by               #
#  the Free Software Foundation, either version 3 of the License, or                  #
#  (at your option) any later version.                                                #
#                                                                                     #
#  This program is distributed in the hope that it will be useful,                    #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of                     #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                      #
#  GNU General Public License for more details.                                       #
#                                                                                     #
#  You should have received a copy of the GNU General Public License                  #
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.              #
# *********************************************************************************** #
include(CheckIncludeFile)
cmake_minimum_required(VERSION 3.6)

INSTALL(
  FILES yardControl.cfg
  DESTINATION etc
)

INSTALL(
  FILES yardcontrol
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_EXECUTE WORLD_READ
  DESTINATION etc/init.d
)

add_subdirectory(openhab2)
