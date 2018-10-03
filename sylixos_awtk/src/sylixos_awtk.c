#include <stdio.h>

extern int gui_app_start(int lcd_w, int lcd_h);


int main (int argc, char **argv)
{
    gui_app_start(800, 480);

    return  (0);
}
