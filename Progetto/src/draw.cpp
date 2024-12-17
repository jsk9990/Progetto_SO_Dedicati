// draw_utils.cpp
#include"config.h"
#include "draw.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

void draw(ALLEGRO_FONT *font, float x, float y, int priorita)
{
    al_draw_textf(font, al_map_rgb(255, 255, 255), x, y, ALLEGRO_ALIGN_LEFT, "Priorita: %d", priorita);
}

void draw_priority(ALLEGRO_FONT *small_font, float x, float y, int priority)
{
    al_draw_textf(small_font, al_map_rgb(255, 255, 255), x, y, ALLEGRO_ALIGN_CENTER, "P: %d", priority);
}

void draw_scheduler(ALLEGRO_FONT *small_font, float x, float y, const std::string& scheduler_type)
{
    al_draw_textf(small_font, al_map_rgb(255, 255, 255), x, y, ALLEGRO_ALIGN_CENTER, "SCHEDULER: %s", scheduler_type.c_str());
}

void draw_square(float x, float y, float size, ALLEGRO_COLOR color)
{
    al_draw_filled_rectangle(x, y, x + size, y + size, color);
}

void draw_circle(float x, float y, float size, ALLEGRO_COLOR color)
{
    al_draw_filled_circle(x + size / 2, y + size / 2, size / 2, color);
}

void draw_triangle(float x, float y, float size, ALLEGRO_COLOR color)
{
    al_draw_filled_triangle(x, y + size, x + size / 2, y, x + size, y + size, color);
}

void draw_menu(ALLEGRO_FONT *font, ALLEGRO_FONT *small_font)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));
    if (menu_state == SELECT_ALGORITHM)
    {
        al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 150, ALLEGRO_ALIGN_CENTER, "GESTIONE DEI THREAD CONCORRENTI");
        al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 110, ALLEGRO_ALIGN_CENTER, "(Il programma lavora con un solo core di default)");
        al_draw_text(small_font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 30, ALLEGRO_ALIGN_CENTER, "Selezionare il numero di figure da creare da 1 a 9 premendo sulla tastiera");
    }
}

void draw_performance_metrics(ALLEGRO_FONT *font, int context_switches)
{

    al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, "Premere ESC per terminare");
    al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH - 10, 10, ALLEGRO_ALIGN_RIGHT, "Context Switches: %d", context_switches);
}
