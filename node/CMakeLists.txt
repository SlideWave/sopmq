# SOPMQ - Scalable optionally persistent message queue
# Copyright 2014 InWorldz, LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

PROJECT( sopmq-node )
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Boost COMPONENTS thread system REQUIRED)
include_directories(${PORTAUDIO_INCLUDE_DIRS} ${Boost_INCLUDE_DIR} )

set(LIBS ${LIBS} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})

file(GLOB LIB_SRCS src/*.h src/*.cpp)
	
ADD_EXECUTABLE(sopmq-node ${LIB_SRCS})
target_link_libraries(sopmq-node ${LIBS})