include config/finalcut

$(if $(shell test -e $(fc_include_path)/final/final.h || echo -n no),\
    $(eval $(this).ignore := 1) \
    $(info **) \
    $(info ** library 'finalcut' not found) \
    $(info ** make sure 'finalcut' is installed ) \
    $(info ** and paths in 'config/finalcut' are correct) \
    $(info **) \
)

$(this).requires.lib := \
    ulm2/libulm2.a

$(this).CXXFLAGS := -I $(fc_include_path) -std=c++17
$(this).LDFLAGS := -Wl,-rpath $(fc_lib_path) -L $(fc_lib_path) -lfinal -ltermcap

$(this).prg.cpp := \
    udb-tui.cpp

$(this).install := \
    udb-tui
