cmake_minimum_required(VERSION 3.5)

file(READ "src/version.h" vercontents)
string(REGEX MATCH "VERSION_MAJOR ([0-9]*)" _ ${vercontents})
set(ver_major ${CMAKE_MATCH_1})
string(REGEX MATCH "VERSION_MINOR ([0-9]*)" _ ${vercontents})
set(ver_minor ${CMAKE_MATCH_1})
string(REGEX MATCH "VERSION_BUILD ([0-9]*)" _ ${vercontents})
set(ver_build ${CMAKE_MATCH_1})
set(sem_ver "${ver_major}.${ver_minor}.${ver_build}")

message(STATUS "Building OpenKJ Songbook Agent ${sem_ver}")

message("Building OpenKJ Songbook Agent ${sem_ver}")

project(SongbookAgent
        VERSION ${sem_ver}
        LANGUAGES CXX C
        )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Sql Network Widgets Concurrent Svg PrintSupport REQUIRED)

include_directories(
        src/
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	message("Linux or BSD detected")
	# Linux/BSD specific stuff
endif ()


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("macOS detected")
    # macOS specific stuff
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	message("Windows detected")
	# Windows specific stuff
   	set(WINDOWS_ICON_RC "appicon.rc")
endif ()

set(SOURCE_FILES
        src/dialogabout.cpp
	src/dialogabout.h
	src/dialogabout.ui
	src/dialogsettings.cpp
	src/dialogsettings.h
	src/dialogsettings.ui
	src/dialogupdate.cpp
	src/dialogupdate.h
	src/dialogupdater.cpp
	src/dialogupdater.h
	src/dialogupdater.ui
	src/dialogupdate.ui
	src/main.cpp
	src/okjsongbookapi.cpp
	src/okjsongbookapi.h
	src/requeststablemodel.cpp
	src/requeststablemodel.h
	src/settings.cpp
	src/settings.h
	src/songbookclient.cpp
	src/songbookclient.h
	src/songbookclient.ui
	src/version.h
	resources.qrc
	)

set(LIBRARIES
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
        Qt5::Network
        Qt5::Svg
        Qt5::Concurrent
        )


if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	add_executable(SongbookAgent
            ${SOURCE_FILES}
            )
    target_link_libraries(SongbookAgent ${LIBRARIES})

    install(
	    TARGETS SongbookAgent 
            DESTINATION bin
    )
    install(
	    FILES src/Icons/org.openkj.SongbookAgent.svg
            DESTINATION share/icons/hicolor/scalable/apps
    )
    install(
	    FILES src/org.openkj.SongbookAgent.desktop
            DESTINATION share/applications
    )
    install(
	    FILES src/org.openkj.SongbookAgent.metainfo.xml
            DESTINATION share/metainfo
    )
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(openkj_ICON ${CMAKE_CURRENT_SOURCE_DIR}/src/Icons/OpenKJ.icns)
    set_source_files_properties(${openkj_ICON} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
            )
    add_executable(SongbookAgent
            MACOSX_BUNDLE
            ${openkj_ICON}
            ${SOURCE_FILES}
            )
    set_target_properties(SongbookAgent PROPERTIES
            BUNDLE True
            MACOSX_BUNDLE_GUI_IDENTIFIER org.openkj.SongbookAgent
            MACOSX_BUNDLE_BUNDLE_NAME SongbookAgent
            MACOSX_BUNDLE_BUNDLE_VERSION ${sem_ver}
            MACOSX_BUNDLE_ICON_FILE OpenKJ.icns
            )
    target_link_libraries(SongbookAgent
            ${LIBRARIES}
            )
    if (NOT DEFINED BUILDONLY)
        get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)
        execute_process(
                COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
                RESULT_VARIABLE return_code
                OUTPUT_VARIABLE qt5_install_prefix
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (qt5_install_prefix)
            find_program(MACDEPLOYQT macdeployqt HINTS ${qt5_install_prefix} PATH_SUFFIXES bin)
            if (MACDEPLOYQT)
                if (DEFINED MAC_SIGNING_IDENTITY)
                    add_custom_command(
                            TARGET SongbookAgent
                            COMMAND "${MACDEPLOYQT}"
                            VERBATIM
                            ARGS
                            "${CMAKE_BINARY_DIR}/SongbookAgent.app"
                            -codesign=${MAC_SIGNING_IDENTITY}
                            "-appstore-compliant"
                    )
                else ()
                    add_custom_command(
                            TARGET SongookAgent
                            COMMAND "${MACDEPLOYQT}"
                            ARGS "${CMAKE_BINARY_DIR}/SongbookAgent.app"
                    )
                endif ()
            else ()
                message("Can't find macdeployqt")
            endif ()
        else ()
            message("QT_INSTALL_DIR not defined")
        endif ()
    endif ()
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_executable(SongbookAgent
            ${SOURCE_FILES}
            ${WINDOWS_ICON_RC}
            )
    target_link_libraries(SongbookAgent PRIVATE ${LIBRARIES})
    set_property(
            TARGET SongbookAgent
            PROPERTY WIN32_EXECUTABLE true
    )

    if (DEFINED DEPLOY_DEPS)
        get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)
        execute_process(
                COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
                RESULT_VARIABLE return_code
                OUTPUT_VARIABLE qt5_install_prefix
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (qt5_install_prefix)
            find_program(WINDEPLOYQT windeployqt HINTS ${qt5_install_prefix} PATH_SUFFIXES bin)
            if (WINDEPLOYQT)
                add_custom_command(
                        TARGET SongbookAgent
                        COMMAND "${WINDEPLOYQT}"
			ARGS "${CMAKE_BINARY_DIR}\\SongbookAgent.exe"
                )
            else ()
                message("Can't find windeployqt")
            endif ()
        else ()
            message("QT_INSTALL_DIR not defined")
        endif ()
    endif ()
endif ()


