#ifndef CONFIG_UTILS_H
#define CONFIG_UTILS_H

inline constexpr int SCREEN_WIDTH = 800;
inline constexpr int SCREEN_HEIGHT = 1024;
inline constexpr int SHAPE_SIZE = 35;
inline constexpr int MENU_HEIGHT = 100;
extern int NUM_SHAPES;

extern int current_policy;
extern bool policy_selected;
extern int num_cores; // Default to 1 core

enum MenuState
{
    SELECT_ALGORITHM
};

extern MenuState menu_state; 

#endif