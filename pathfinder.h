#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <gtk/gtk.h>
#include <stdbool.h>

#define MAX_LEVELS 5
#define MAX_GRID_SIZE 20
#define LEVEL_FILE "level_data.txt"
#define SCORE_FILE "high_score.txt"

// --- Game state ---
typedef enum {
    GAME_STATE_RUNNING,
    GAME_STATE_WIN_LEVEL,
    GAME_STATE_GAME_OVER
} GameState;

// --- Level structure ---
typedef struct {
    char grid[MAX_GRID_SIZE][MAX_GRID_SIZE];
    int rows;
    int cols;
    double time_limit; // seconds
} Level;

// --- Game data ---
typedef struct {
    Level levels[MAX_LEVELS];
    int current_level_idx;
    GameState state;
    double player_size;
    int player_grid_x;
    int player_grid_y;
    gint64 start_time_us;
    double elapsed_time;
    int score;
    int high_score;

    gboolean is_w_pressed;
    gboolean is_s_pressed;
    gboolean is_a_pressed;
    gboolean is_d_pressed;

    GtkWidget *window;
    GtkWidget *drawing_area;
    GtkWidget *status_label;
    GtkWidget *message_label;
} GameData;

// Global
extern GameData game_data;

// --- Logic functions ---
gboolean load_all_levels(void);
void start_level(int level_idx);
void game_over(const char *reason);
void load_high_score(void);
void save_high_score(void);
void update_status_labels(void);

#endif
