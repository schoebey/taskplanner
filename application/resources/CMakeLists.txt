if(UNIX AND NOT APPLE)
    install(FILES org.ancor.taskplanner.desktop DESTINATION share/applications/ COMPONENT desktop)
    install(FILES taskplanner.png DESTINATION share/icons/hicolor/256x256/apps COMPONENT desktop)
endif()
