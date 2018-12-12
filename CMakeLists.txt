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

project (yardControl)
set (CMAKE_C_FLAGS "-std=gnu11 -Wall")

find_library(LIB_MQTT   mosquitto)
find_library(LIB_WIRING wiringPi)

# all executables end up in bin
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_executable(yardControl yardControl.c pushButton.c readConfig.c logging.c daemon.c mqttGateway.c)

target_link_libraries(yardControl "${LIB_MQTT}")
target_link_libraries(rpisensorclient "${LIB_WIRING}")

set(CMAKE_INSTALL_PREFIX /)
INSTALL(PROGRAMS bin/yardControl DESTINATION usr/sbin)
add_subdirectory(Contrib)
