
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
#include <draw.h> 
#include <struct.h>
#include <config.h>


void *move_shape(void *arg)
{
    Shape *shape = (Shape *)arg;
    float half_screen = SCREEN_WIDTH / 2 - SHAPE_SIZE;
    float start_x = shape->half == 0 ? 0 : SCREEN_WIDTH / 2 - SHAPE_SIZE;
    float end_x = shape->half == 0 ? half_screen : SCREEN_WIDTH - SHAPE_SIZE;

    time_t start_time = time(nullptr);

    // Aggiungiamo una variabile per tenere traccia della direzione
    bool moving_right = shape->half == 0;  // True se la forma si sta muovendo a destra, false per sinistra

    
    while (true)
    {
        for (volatile int i = 0; i < 100000000; ++i)
        {
            // SI PUÒ MODIFICARE CARICO LAVORO PER VEDERE DIFFERENZE
        }

        // Ogni volta che il thread è schedulato, la posizione viene aggiornata

        shape->x += (moving_right ? 1 : -1);  // Sposta la forma a destra o a sinistra

        // Controlla se la forma ha raggiunto i limiti del percorso
        if (shape->x <= start_x || shape->x >= end_x)
        {
            // Inverte la direzione
            moving_right = !moving_right;
        }

        // Calcola il tempo trascorso e cede volontariamente il controllo al sistema operativo ogni secondo
        if (difftime(time(nullptr), start_time) >= 1)
        {
            sched_yield();  // Cede il controllo al sistema operativo
            start_time = time(nullptr);  // Reset del timer
        }

        usleep(41666);  // Pausa per 24fps
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
        return context_switches;
    }

    // Misura il tempo corrente
    time_t current_time = time(NULL);

    
    // Se sono passati 10 secondi, calcola la differenza
    if (difftime(current_time, start_time) >= 10.0)
    {
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

    // Create display
    ALLEGRO_DISPLAY *display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!display)
    {
        std::cerr << "Failed to create display!" << std::endl;
        return -1;
    }

    // Load font
    ALLEGRO_FONT *font = al_load_ttf_font("/home/jsk/Scrivania/Progetto_SO_Dedicati/Progetto/Font/Pixel.TTF", 24, 0);
    ALLEGRO_FONT *small_font = al_load_ttf_font("/home/jsk/Scrivania/Progetto_SO_Dedicati/Progetto/Font/Minecraft.ttf", 12, 0);
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

    // Select initial scheduling policy
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    bool policy_selected = false;

    while (!policy_selected)
    {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        draw_menu(font, small_font);
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
                    NUM_SHAPES = 1;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_2:
                    NUM_SHAPES = 2;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_3:
                    NUM_SHAPES = 3;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_4:
                    NUM_SHAPES = 4;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_5:
                    NUM_SHAPES = 5;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_6:
                    NUM_SHAPES = 6;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_7:
                    NUM_SHAPES = 7;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_8:
                    NUM_SHAPES = 8;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_9:
                    NUM_SHAPES = 9;
                    policy_selected = true;
                    break;
                case ALLEGRO_KEY_0:
                    NUM_SHAPES = 10;
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
        shapes.push_back({static_cast<float>(SCREEN_WIDTH) / 2, y, speed2, 1, priority, color2, draw_func2});
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

        // Assegna una schedulazione diversa a ciascun thread

        struct sched_param param = {}; // Inizializza senza impostare sched_priority
        int policy;
        switch (i % 3)
        {
        case 0:
            policy = SCHED_FIFO;
            param.sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;//MODIFICARE PRIORITÀ PER VEDERE DEI CAMBIAMENTI 
            shapes[i].scheduler_type = "SCHED_FIFO";
            break;
        case 1:
            policy = SCHED_RR;
            param.sched_priority = sched_get_priority_min(SCHED_RR) + 3;//MODIFICARE PRIORITÀ PER VEDERE DEI CAMBIAMENTI
            shapes[i].scheduler_type = "SCHED_RR";
            break;
        default:
            policy = SCHED_OTHER;
            shapes[i].scheduler_type = "SCHED_OTHER";
            break;
        }

        //Controllo sulla priorità per SCHED FIFO e SCHED RR 
        

        if (pthread_setschedparam(threads[i], policy, &param) != 0)
        {
            std::cerr << "Errore nell'impostazione dello scheduler per il thread " << i << std::endl;
        }else {
            std::cout << "Thread " << i << " configurato con " << shapes[i].scheduler_type << std::endl;
            std::cout << "Priorità impostata: " << param.sched_priority << std::endl;
        }

        int cur_policy;
        struct sched_param cur_param;
        pthread_getschedparam(threads[i], &cur_policy, &cur_param);
        std::cout << "Thread " << i << " ha la politica " 
                << ((cur_policy == SCHED_FIFO) ? "SCHED_FIFO" :
                    (cur_policy == SCHED_RR) ? "SCHED_RR" : "SCHED_OTHER")
                << " e priorità " << cur_param.sched_priority << std::endl;


    }

    // Initialize timing variables
    start_time = time(nullptr);

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

            sem_wait(&render_semaphore);
            al_clear_to_color(al_map_rgb(0, 0, 0));
            for (const auto &shape : shapes)
                {
                    shape.draw_func(shape.x, shape.y, SHAPE_SIZE, shape.color);
                    draw_priority(small_font, shape.x, shape.y, shape.priority);
                    draw_scheduler(small_font, shape.x, shape.y, shape.scheduler_type);
                }

            draw_performance_metrics(small_font, get_context_switches() - context_switches);
            al_flip_display();

            sem_post(&render_semaphore);

            usleep(16000);

                // Check if 1.5 seconds have passed to check CPU usage
                
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

