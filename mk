#!/bin/sh
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

# build on remote machine
#build_host=192.168.101.10
build_host=192.168.100.201
build_dir=yardControl
build_branch=master

# git repository to get source from
git_repo=192.168.100.26:git/yardControl

# commit and push source locally, retrieve it remptely and build it
git checkout $build_branch;\
git commit . -m "update";\
git push $git_repo &&\
ssh -x $build_host "(cd $build_dir && git checkout $build_branch && git pull && cmake . && make )"
