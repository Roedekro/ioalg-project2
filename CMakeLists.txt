cmake_minimum_required(VERSION 3.6)
project(ioproject2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp InputStreamA.h InputStreamA.cpp OutputStreamA.h OutputStreamA.cpp OutputStreamB.cpp OutputStreamB.h InputStreamB.cpp InputStreamB.h OutputStreamC.cpp OutputStreamC.h InputStreamC.cpp InputStreamC.h InputStreamD.cpp InputStreamD.h OutputStreamD.cpp OutputStreamD.h OutputStream.h InputStream.h Binary.cpp Binary.h BinElement.cpp BinElement.h Quicksort.cpp Quicksort.h)
add_executable(ioproject2 ${SOURCE_FILES} OutputStream.h InputStream.h Binary.cpp Binary.h BinElement.cpp BinElement.h Quicksort.cpp Quicksort.h)