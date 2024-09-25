// LIBRERIE NECESSARIE//
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <vector>
#include <ctime>
#include <csignal>
#include <semaphore.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

// DICHIARAZIONI COSTANTI//

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 1024;
const int SHAPE_SIZE = 35;
const int MENU_HEIGHT = 100;
const int NUM_SHAPES = 4;
int current_policy;
bool policy_selected = false;
int num_cores = 1; // Default to 1 core

enum MenuState
{
    SELECT_ALGORITHM
};

MenuState menu_state = SELECT_ALGORITHM; // possibile aggiunta in futuro per selezionare il numero di thread dal menù

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
    // Funzione di disegno
    void (*draw_func)(float, float, float, ALLEGRO_COLOR);
};

//---------- Funzioni di disegno per le figure ----------
void draw_priority(ALLEGRO_FONT *small_font, float x, float y, int priority)
{
    al_draw_textf(small_font, al_map_rgb(255, 255, 255), x, y, ALLEGRO_ALIGN_CENTER, "P: %d", priority);
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

void *move_shape(void *arg)
{
    Shape *shape = (Shape *)arg;
    float half_screen = SCREEN_WIDTH / 2 - SHAPE_SIZE;
    float start_x = shape->half == 0 ? 0 : SCREEN_WIDTH / 2 - SHAPE_SIZE;
    float end_x = shape->half == 0 ? half_screen : SCREEN_WIDTH - SHAPE_SIZE;

    pthread_t thread = pthread_self();
    std::cout << "Thread ID: " << thread << std::endl;

    int core_id = sched_getcpu();
    std::cout << "Thread running on core: " << core_id << std::endl;

    time_t start_time = time(nullptr);

    while (true)
    {
        shape->x += shape->speed;

        // Check if shape has reached the path limits
        if (shape->x < start_x || shape->x > end_x)
        {
            shape->speed = -shape->speed; // Reverse speed
            shape->x += shape->speed;     // Move shape back within limits
        }

        // Calculate elapsed time
        if (difftime(time(nullptr), start_time) >= 1)
        {
            sched_yield();
            start_time = time(nullptr);
        }
        usleep(33333); // Sleep for roughly 16ms (60fps)
    }

    return nullptr;
}

void set_thread_affinity(pthread_t thread, int total_cores, int thread_index)
{
    int core_id = 0; // così teoricamente assegna la stessa cpu a tutti i thread creati

    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(core_id, &cpu_set);

    int result = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpu_set);
    if (result != 0)
    {
        std::cerr << "Failed to set thread affinity: " << strerror(result) << std::endl;
    }
}

sem_t render_semaphore;
bool running = true;
time_t start_time;
time_t last_policy_change_time;

const char *get_policy_name(int policy)
{
    switch (policy)
    {
    case SCHED_FIFO:
        return "SCHED_FIFO";
    case SCHED_RR:
        return "SCHED_RR";
    case SCHED_OTHER:
        return "SCHED_OTHER";
    default:
        return "UNKNOWN";
    }
}

void draw_menu(ALLEGRO_FONT *font)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));
    if (menu_state == SELECT_ALGORITHM)
    {
        al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 150, ALLEGRO_ALIGN_CENTER, "GESTIONE DEI THREAD CONCORRENTI");
        al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 110, ALLEGRO_ALIGN_CENTER, "(Il programma lavora con un solo core di default)");
        al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 30, ALLEGRO_ALIGN_CENTER, "Select Scheduling Algorithm:");
        al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, ALLEGRO_ALIGN_CENTER, "1. FIFO");
        al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 30, ALLEGRO_ALIGN_CENTER, "2. Round Robin");
        al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 60, ALLEGRO_ALIGN_CENTER, "3. Other");
    }
}

void draw_performance_metrics(ALLEGRO_FONT *font, int current_policy, int context_switches, double cpu_usage)
{
    double elapsed_time = difftime(time(nullptr), start_time);
    double policy_elapsed_time = difftime(time(nullptr), last_policy_change_time);

    al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, "Premere ESC per terminare");

    al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH - 10, 10, ALLEGRO_ALIGN_RIGHT, "Policy: %s", get_policy_name(current_policy));
    al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH - 10, 30, ALLEGRO_ALIGN_RIGHT, "Elapsed Time: %.2f s", elapsed_time);
    al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH - 10, 50, ALLEGRO_ALIGN_RIGHT, "Policy Time: %.2f s", policy_elapsed_time);
    al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH - 10, 70, ALLEGRO_ALIGN_RIGHT, "Context Switches: %d", context_switches);
}

int get_context_switches()
{
    static int previous_context_switches = -1; // Per tenere traccia del valore precedente
    static time_t start_time = 0;              // Per tenere traccia del tempo iniziale

    // Se è la prima volta che la funzione viene chiamata, inizializza il timer
    if (start_time == 0)
    {
        start_time = time(NULL);
    }

    // Comportamento originale della funzione: leggi i context switches da /proc/self/status
    FILE *file = fopen("/proc/self/status", "r");
    if (!file)
        return -1;

    char line[256];
    int context_switches = -1;
    while (fgets(line, sizeof(line), file))
    {
        if (sscanf(line, "voluntary_ctxt_switches: %d", &context_switches) == 1)
        {
            break;
        }
    }
    fclose(file);

    // Se il valore iniziale non è stato ancora salvato, lo salvi
    if (previous_context_switches == -1)
    {
        previous_context_switches = context_switches;
    }

    // Misura il tempo corrente
    time_t current_time = time(NULL);

    int switches_in_last_10_seconds = 0;
    // Se sono passati 10 secondi, calcola la differenza
    if (difftime(current_time, start_time) >= 10.0)
    {
        switches_in_last_10_seconds = context_switches - previous_context_switches;
        char policy_corrente[12] = "";
        switch (current_policy)
        {
        case 1:
            strcpy(policy_corrente, "SCHED_FIFO");
            break;
        case 2:
            strcpy(policy_corrente, "SCHED_RR");
            break;
        case 3:
            strcpy(policy_corrente, "SCHED_OTHER");
            break;
        default:
            strcpy(policy_corrente, "UNKNOWN");
            break;
        }
        printf("Context switches negli ultimi 10 secondi: %d, Policy corrente: %s\n", switches_in_last_10_seconds, policy_corrente);

        // Aggiorna il valore e resetta il timer
        previous_context_switches = context_switches;
        start_time = current_time;
    }

    // Ritorna il valore dei context switches attuali (comportamento originale)
    return context_switches;
}

void print_thread_info(pthread_t thread)
{
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);

    int result = pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpu_set);
    if (result != 0)
    {
        std::cerr << "Failed to get thread affinity: " << strerror(result) << std::endl;
        return;
    }

    bool first = true;
    for (int i = 0; i < CPU_SETSIZE; ++i)
    {
        if (CPU_ISSET(i, &cpu_set))
        {
            if (!first)
                std::cout << ", ";
            std::cout << i;
            first = false;
        }
    }
    std::cout << std::endl;
}

double get_cpu_usage()
{
    static unsigned long long last_total_user, last_total_user_low, last_total_sys, last_total_idle;
    float percent;

    FILE *file;
    unsigned long long total_user, total_user_low, total_sys, total_idle, total;

    file = fopen("/proc/stat", "r");

    if (file == nullptr)
        return -1;

    fscanf(file, "cpu %llu %llu %llu %llu", &total_user, &total_user_low, &total_sys, &total_idle);
    fclose(file);

    if (total_user < last_total_user || total_user_low < last_total_user_low || total_sys < last_total_sys || total_idle < last_total_idle)
    {
        // Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else
    {
        total = (total_user - last_total_user) + (total_user_low - last_total_user_low) + (total_sys - last_total_sys);
        percent = total;
        total += (total_idle - last_total_idle);
        percent /= total;
        percent *= 100;
    }

    last_total_user = total_user;
    last_total_user_low = total_user_low;
    last_total_sys = total_sys;
    last_total_idle = total_idle;

    return percent;
}

int main()
{
    // Initialize primitives addon
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    // Initialize Allegro
    if (!al_init() || !al_init_primitives_addon() || !al_install_keyboard() || !al_init_font_addon() || !al_init_ttf_addon())
    {
        std::cerr << "Failed to initialize Allegro or its addons" << std::endl;
        return -1;
    }

    // Install signal handler
    // signal(SIGINT, signal_handler);

    // Create display
    ALLEGRO_DISPLAY *display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!display)
    {
        std::cerr << "Failed to create display!" << std::endl;
        return -1;
    }

    // Load font
    ALLEGRO_FONT *font = al_load_ttf_font("../Font/Pixel.TTF", 24, 0);
    ALLEGRO_FONT *small_font = al_load_ttf_font("../Font/Minecraft.ttf", 12, 0);
    if (!font || !small_font)
    {
        std::cerr << "Failed to load font!" << std::endl;
        al_destroy_display(display);
        return -1;
    }

    // Install keyboard
    if (!al_install_keyboard())
    {
        std::cerr << "Failed to install keyboard!" << std::endl;
        al_destroy_display(display);
        return -1;
    }

    // Initialize semaphore
    sem_init(&render_semaphore, 0, 1);

    // inizializzo menù

    // Select initial scheduling policy
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    bool policy_selected = false;

    while (!policy_selected)
    {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        draw_menu(font);
        al_flip_display();

        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            switch (menu_state)
            {
            case SELECT_ALGORITHM:
                switch (ev.keyboard.keycode)
                {
                case ALLEGRO_KEY_1:
                    current_policy = SCHED_FIFO;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_2:
                    current_policy = SCHED_RR;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_3:
                    current_policy = SCHED_OTHER;
                    policy_selected = true;
                    break;
                }
                break;
            }
        }
    }

    // Define shapes
    std::vector<Shape> shapes;
    for (int i = 0; i < NUM_SHAPES / 2; ++i)
    {
        float y = MENU_HEIGHT + (i * 2 * (SCREEN_HEIGHT - MENU_HEIGHT) / NUM_SHAPES);
        float speed1 = (i % 2 == 0) ? 2.0f : 3.0f;
        float speed2 = ((i + 1) % 2 == 0) ? 2.0f : 3.0f;
        int priority = rand() % 10 + 1; // Assegna una priorità casuale tra 1 e 10
        ALLEGRO_COLOR color1 = al_map_rgb(rand() % 256, rand() % 256, rand() % 256);
        ALLEGRO_COLOR color2 = al_map_rgb(rand() % 256, rand() % 256, rand() % 256);
        void (*draw_func1)(float, float, float, ALLEGRO_COLOR) = (i % 3 == 0) ? draw_square : (i % 3 == 1) ? draw_circle
                                                                                                           : draw_triangle;
        void (*draw_func2)(float, float, float, ALLEGRO_COLOR) = ((i + 1) % 3 == 0) ? draw_square : ((i + 1) % 3 == 1) ? draw_circle
                                                                                                                       : draw_triangle;

        // Create shape for the left half
        shapes.push_back({0, y, speed1, 0, priority, color1, draw_func1});

        // Create shape for the right half
        shapes.push_back({SCREEN_WIDTH / 2, y, speed2, 1, priority, color2, draw_func2});
    }

    struct sched_param param;
    param.sched_priority = 10;

    // Create threads
    std::vector<pthread_t> threads(NUM_SHAPES);
    for (int i = 0; i < NUM_SHAPES; ++i)
    {
        pthread_create(&threads[i], nullptr, move_shape, &shapes[i]);
        set_thread_affinity(threads[i], num_cores, i);
        print_thread_info(threads[i]);
        pthread_setschedparam(threads[i], current_policy, &param);
    }

    if (running)
    {
        struct sched_param param;
        param.sched_priority = (current_policy == SCHED_OTHER) ? 0 : 10;
        for (const auto &thread : threads)
        {
            if (pthread_setschedparam(thread, current_policy, &param) != 0)
            {
                std::cerr << "Failed to set initial scheduling policy" << std::endl;
            }
        }
    }

    // Initialize timing variables
    start_time = time(nullptr);
    last_policy_change_time = time(nullptr);
    int context_switches = get_context_switches();

    time_t last_cpu_check_time = time(nullptr);

    // Main loop
    while (running)
    {
        ALLEGRO_EVENT ev;
        bool event_occured = false;

        while ((al_get_next_event(event_queue, &ev)))
        {
            if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                running = false;
            }
        }

        if (!event_occured)
        {

            if (!policy_selected)
            {
                draw_menu(font);
            }
            else
            {
                sem_wait(&render_semaphore);
                al_clear_to_color(al_map_rgb(0, 0, 0));
                for (const auto &shape : shapes)
                {
                    shape.draw_func(shape.x, shape.y, SHAPE_SIZE, shape.color);
                    draw_priority(small_font, shape.x, shape.y, shape.priority);
                }
                draw_performance_metrics(font, current_policy, get_context_switches() - context_switches, get_cpu_usage());
                al_flip_display();

                sem_post(&render_semaphore);

                usleep(16000);

                // Check if 1.5 seconds have passed to check CPU usage
                if (difftime(time(nullptr), last_cpu_check_time) >= 1.5)
                {
                    last_cpu_check_time = time(nullptr);
                    float cpu_usage = get_cpu_usage();
                    std::cout << "CPU Usage: " << cpu_usage << "%" << std::endl;
                }
            }
        }
    }

    // Cleanup
    for (auto &thread : threads)
    {
        pthread_cancel(thread);
        pthread_join(thread, nullptr);
    }

    al_destroy_display(display);
    al_destroy_font(font);
    al_destroy_event_queue(event_queue);
    sem_destroy(&render_semaphore);

    return 0;
}
