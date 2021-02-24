cmake_minimum_required(VERSION 3.2)

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
	${FL_PATH}/http/http.cpp
	${FL_PATH}/http/http11_parser.cpp
	${FL_PATH}/http/httpclient_parser.cpp
	${FL_PATH}/http/http_parser.cpp
	${FL_PATH}/http/http_session.cpp
	)
SET(T_LIB
	dl
	yaml-cpp
	pthread
	)

#include_directories(../src/FL)

project(exampleLog)
add_executable(${PROJECT_NAME} ./exampleLog.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleThread)
add_executable(${PROJECT_NAME} ./exampleThread.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleCoroutine)
add_executable(${PROJECT_NAME} ./exampleCoroutine.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleCoroutine2)
add_executable(${PROJECT_NAME} ./exampleCoroutine2.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleScheduler)
add_executable(${PROJECT_NAME} ./exampleScheduler.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleiomanager)
add_executable(${PROJECT_NAME} ./exampleiomanager.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleTimer)
add_executable(${PROJECT_NAME} ./exampleTimer.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleHook)
add_executable(${PROJECT_NAME} ./exampleHook.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleAddress)
add_executable(${PROJECT_NAME} ./exampleAddress.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleSocket)
add_executable(${PROJECT_NAME} ./exampleSocket.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleByteArray)
add_executable(${PROJECT_NAME} ./exampleByteArray.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleHttp)
add_executable(${PROJECT_NAME} ./exampleHttp.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleHttpParser)
add_executable(${PROJECT_NAME} ./exampleHttpParser.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleTcpserver)
add_executable(${PROJECT_NAME} ./exampleTcpserver.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

project(exampleEcho)
add_executable(${PROJECT_NAME} ./exampleEcho.cpp ${SRC})
target_link_libraries(${PROJECT_NAME} ${T_LIB})

