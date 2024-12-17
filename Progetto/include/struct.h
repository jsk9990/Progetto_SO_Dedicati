// struct.h
#ifndef STRUCT_H
#define STRUCT_H

#include <allegro5/allegro.h>
#include <string>  // Per la gestione delle stringhe


struct Shape
{
    // Coordinate della figura
    float x, y;
    // Velocità
    float speed;
    // 0 = sinistra, 1 = destra
    int half;
    // Priorità
    int priority;
    // Colore
    ALLEGRO_COLOR color;
    // Stringa che riporta il tipo di schedulazione
    std::string scheduler_type;
    // Funzione di disegno
    void (*draw_func)(float, float, float, ALLEGRO_COLOR);

    // Costruttore
    Shape(float x_, float y_, float speed_, int half_, int priority_, 
          ALLEGRO_COLOR color_, void (*draw_func_)(float, float, float, ALLEGRO_COLOR))
        : x(x_), y(y_), speed(speed_), half(half_), priority(priority_), 
          color(color_), scheduler_type(""), draw_func(draw_func_) {};

};

#endif 