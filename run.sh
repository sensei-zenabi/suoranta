gcc main.c -o peli \
    $(sdl2-config --cflags --libs) \
    -lSDL2_ttf \
    -lSDL2_gfx \
    -lm
./peli
