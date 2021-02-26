cmake_minimum_required(VERSION 3.2)

project(FL)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build/)
SET(CMAKE_BUILD_TYPE debug)
SET(CMKAE_CXX_FLAGS_DEBUG -g)
SET(FL_PATH ../src/FL)
SET(SRC
	${FL_PATH}/logmanager.cpp
	${FL_PATH}/log.cpp
	${FL_PATH}/util.cpp
	${FL_PATH}/config.cpp
	${FL_PATH}/thread.cpp
	${FL_PATH}/coroutine.cpp
	${FL_PATH}/scheduler.cpp
	${FL_PATH}/iomanager.cpp
	${FL_PATH}/timer.cpp
	${FL_PATH}/hook.cpp
	${FL_PATH}/fd_manager.cpp
	${FL_PATH}/address.cpp
	${FL_PATH}/socket.cpp
	${FL_PATH}/bytearray.cpp
	${FL_PATH}/tcp_server.cpp
	${FL_PATH}/stream.cpp
	${FL_PATH}/socket_stream.cpp
	${FL_PATH}/uri.cpp
	${FL_PATH}/http/http.cpp
	${FL_PATH}/http/http11_parser.cpp
	${FL_PATH}/http/httpclient_parser.cpp
	${FL_PATH}/http/http_parser.cpp
	${FL_PATH}/http/http_session.cpp
	${FL_PATH}/http/http_server.cpp
	${FL_PATH}/http/http_servlet.cpp
	${FL_PATH}/http/http_connection.cpp
	)
add_library(FL SHARED ${SRC})
SET(T_LIB
	dl
	yaml-cpp
	pthread
	)

#include_directories(../src/FL)

link_libraries(FL ${T_LIB})

add_executable(exampleLog ./exampleLog.cpp )
add_executable(exampleThread ./exampleThread.cpp )
add_executable(exampleCoroutine ./exampleCoroutine.cpp )
add_executable(exampleCoroutine2 ./exampleCoroutine2.cpp )
add_executable(exampleScheduler ./exampleScheduler.cpp )
add_executable(exampleiomanager ./exampleiomanager.cpp )
add_executable(exampleTimer ./exampleTimer.cpp )
add_executable(exampleHook ./exampleHook.cpp )
add_executable(exampleAddress ./exampleAddress.cpp )
add_executable(exampleSocket ./exampleSocket.cpp )
add_executable(exampleByteArray ./exampleByteArray.cpp )
add_executable(exampleHttp ./exampleHttp.cpp )
add_executable(exampleHttpParser ./exampleHttpParser.cpp )
add_executable(exampleTcpserver ./exampleTcpserver.cpp )
add_executable(exampleEcho ./exampleEcho.cpp )
add_executable(exampleHttpserver ./exampleHttpserver.cpp )
add_executable(exampleHttpconnection ./exampleHttpconnection.cpp )
add_executable(exampleUri ./exampleUri.cpp )
