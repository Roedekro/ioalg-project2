cmake_minimum_required(VERSION 3.6)
project(ioalg-project2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp InputStreamA.h InputStreamA.cpp OutputStreamA.h OutputStreamA.cpp OutputStreamB.cpp OutputStreamB.h InputStreamB.cpp InputStreamB.h OutputStreamC.cpp OutputStreamC.h InputStreamC.cpp InputStreamC.h InputStreamD.cpp InputStreamD.h OutputStreamD.cpp OutputStreamD.h OutputStream.h InputStream.h Binary.cpp Binary.h BinElement.cpp BinElement.h Quicksort.cpp Quicksort.h ExternalHeapsort.cpp ExternalHeapsort.h MinHeap.cpp MinHeap.h ExternalHeap.cpp ExternalHeap.h Node.cpp Node.h)
add_executable(ioalg-project2 ${SOURCE_FILES} OutputStream.h InputStream.h Binary.cpp Binary.h BinElement.cpp BinElement.h Quicksort.cpp Quicksort.h ExternalHeapsort.cpp ExternalHeapsort.h MinHeap.cpp MinHeap.h ExternalHeap.cpp ExternalHeap.h Node.cpp Node.h)