add_library( openephys okFrontPanelDLL.cpp rhd2000datablock.cpp rhd2000evalboard.cpp rhd2000registers.cpp)

target_link_libraries (openephys dl)
