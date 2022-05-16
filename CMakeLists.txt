# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project (SDS_project)

# Create a sources variable with a link to all cpp files to compile
set (sds_capture_sou 
        src/main/main_SDS_capture.cpp
        src/PckCap/CPckCap.cpp
        src/other/bit_conversion.cpp
)

set (sds_pcap_Sou
    src/main/main_SDS_pcap.cpp
    src/other/bit_conversion.cpp
)

# Add an executable with the above sources
add_executable(SDS_pcap ${sds_pcap_Sou})

target_include_directories(SDS_pcap PRIVATE ${PROJECT_SOURCE_DIR}/include)

IF (WIN32)
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_win/libCsvStorage.a)
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_win/liblibconfig++.dll.a )
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_win/libSketchPoolSE.a ) 
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_win/libPcapSE.a)
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_win/libPacketSE.a)
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_win/libHashSE.a)
    
ELSEIF (UNIX)
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_linux/libCsvStorage.a)
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_linux/libconfig++.so )
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_linux/libSketchPoolSE.a ) 
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_linux/libPcapSE.a)
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_linux/libPacketSE.a)
        target_link_libraries(SDS_pcap ${PROJECT_SOURCE_DIR}/lib_linux/libHashSE.a)

        # Add an executable with the above sources
        add_executable(SDS_capture ${sds_capture_sou}) 

        target_include_directories(SDS_capture PRIVATE ${PROJECT_SOURCE_DIR}/include)

        target_link_libraries(SDS_capture libpcap.a)
        target_link_libraries(SDS_capture ${PROJECT_SOURCE_DIR}/lib_linux/libSketchPoolSE.a)
        target_link_libraries(SDS_capture ${PROJECT_SOURCE_DIR}/lib_linux/libPacketSE.a)
        target_link_libraries(SDS_capture ${PROJECT_SOURCE_DIR}/lib_linux/libHashSE.a)
        target_link_libraries(SDS_capture ${PROJECT_SOURCE_DIR}/lib_linux/libCsvStorage.a)
        target_link_libraries(SDS_capture ${PROJECT_SOURCE_DIR}/lib_linux/libconfig++.so )
        # target_link_libraries(SDS_capture ${PROJECT_SOURCE_DIR}/lib/libRedisStorage.a)
ELSEIF ()
ENDIF ()
