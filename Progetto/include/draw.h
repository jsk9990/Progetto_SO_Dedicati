// draw_utils.h
#ifndef DRAW_UTILS_H
#define DRAW_UTILS_H

#include <string>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_color.h>

// Dichiarazioni delle funzioni
void draw(ALLEGRO_FONT *font, float x, float y, int priorita);
void draw_priority(ALLEGRO_FONT *small_font, float x, float y, int priority);
void draw_scheduler(ALLEGRO_FONT *small_font, float x, float y, const std::string& scheduler_type);
void draw_square(float x, float y, float size, ALLEGRO_COLOR color);
void draw_circle(float x, float y, float size, ALLEGRO_COLOR color);
void draw_triangle(float x, float y, float size, ALLEGRO_COLOR color);
void draw_menu(ALLEGRO_FONT *font,ALLEGRO_FONT *small_font );
void draw_performance_metrics(ALLEGRO_FONT *font,int context_switches);

#endif // DRAW_UTILS_H
