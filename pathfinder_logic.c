#include "pathfinder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glib.h>

// --- Global ---
GameData game_data;

// --- High score ---
void load_high_score(void)
{
    FILE *file = fopen(SCORE_FILE, "r");
    if (file)
    {
        if (fscanf(file, "%d", &game_data.high_score) != 1)
            game_data.high_score = 0;
        fclose(file);
    }
    else
    {
        game_data.high_score = 0;
    }
}

void save_high_score(void)
{
    if (game_data.score > game_data.high_score)
        game_data.high_score = game_data.score;

    FILE *file = fopen(SCORE_FILE, "w");
    if (file)
    {
        fprintf(file, "%d", game_data.high_score);
        fclose(file);
    }
}

// --- Level loading ---
gboolean load_all_levels(void)
{
    FILE *file = fopen(LEVEL_FILE, "r");
    if (!file)
    {
        g_printerr("Error: Could not open level file %s\n", LEVEL_FILE);
        return FALSE;
    }

    char line[1024];
    int current_level = -1;
    int row_count = 0;

    while (fgets(line, sizeof(line), file))
    {
        g_strstrip(line);
        if (line[0] == '\0')
            continue;

        if (strstr(line, "--- Level"))
        {
            current_level++;
            if (current_level >= MAX_LEVELS)
                break;
            row_count = 0;

            if (!fgets(line, sizeof(line), file))
            {
                g_printerr("Error: Missing size/time for level %d\n", current_level + 1);
                fclose(file);
                return FALSE;
            }
            g_strstrip(line);

            int r, c;
            double t;
            if (sscanf(line, "%d,%d,%lf", &r, &c, &t) != 3)
            {
                g_printerr("Error: Invalid size/time for level %d: '%s'\n", current_level + 1, line);
                fclose(file);
                return FALSE;
            }

            Level *level = &game_data.levels[current_level];
            level->rows = r;
            level->cols = c;
            level->time_limit = t;

            continue;
        }

        // grid
        if (current_level >= 0 && current_level < MAX_LEVELS)
        {
            Level *level = &game_data.levels[current_level];
            if (row_count < level->rows)
            {
                for (int j = 0; j < level->cols; j++)
                    level->grid[row_count][j] = (line[j] != '\0') ? line[j] : '.';
                row_count++;
            }
        }
    }

    fclose(file);
    g_print("Levels loaded successfully (%d levels).\n", MAX_LEVELS);
    return TRUE;
}

// --- Start level ---
void start_level(int level_idx)
{
    if (level_idx >= MAX_LEVELS)
    {
        game_data.state = GAME_STATE_WIN_LEVEL;
        gtk_label_set_text(GTK_LABEL(game_data.message_label),
                           "CONGRATULATIONS! You beat all levels!");
        save_high_score();
        return;
    }

    Level *level = &game_data.levels[level_idx];
    gboolean start_found = FALSE;
    for (int i = 0; i < level->rows; i++)
        for (int j = 0; j < level->cols; j++)
            if (level->grid[i][j] == 'S')
            {
                game_data.player_grid_y = i;
                game_data.player_grid_x = j;
                start_found = TRUE;
                break;
            }
    if (!start_found)
    {
        game_over("Level data is missing a start point (S)!");
        return;
    }

    game_data.current_level_idx = level_idx;
    game_data.state = GAME_STATE_RUNNING;
    game_data.elapsed_time = 0.0;
    game_data.is_w_pressed = game_data.is_s_pressed =
        game_data.is_a_pressed = game_data.is_d_pressed = FALSE;
    game_data.start_time_us = g_get_monotonic_time();

    char msg[128];
    snprintf(msg, sizeof(msg), "Level %d started! Use WASD to reach 'E'.", level_idx + 1);
    gtk_label_set_text(GTK_LABEL(game_data.message_label), msg);
    update_status_labels();
}

// --- Game over ---
void game_over(const char *reason)
{
    game_data.state = GAME_STATE_GAME_OVER;
    save_high_score();
    char msg[128];
    snprintf(msg, sizeof(msg), "GAME OVER! %s | Press SPACE to restart Level 1.", reason);
    gtk_label_set_markup(
        GTK_LABEL(game_data.message_label),
        "<span foreground='red'><b>GAME OVER!</b></span>");

    update_status_labels();
    gtk_widget_queue_draw(game_data.drawing_area);
}

// --- Update labels ---
void update_status_labels(void)
{
    char str[128];
    double time_remaining = game_data.levels[game_data.current_level_idx].time_limit - game_data.elapsed_time;

    if (game_data.state == GAME_STATE_GAME_OVER)
    {
        snprintf(str, sizeof(str),
                 "Level %d Failed | Score: %d | High Score: %d",
                 game_data.current_level_idx + 1, game_data.score, game_data.high_score);
    }
    else
    {
        snprintf(str, sizeof(str),
                 "Level %d/%d | Time: %.1fs%s | Score: %d | High Score: %d",
                 game_data.current_level_idx + 1, MAX_LEVELS,
                 time_remaining, (time_remaining < 0.0 ? " (Over)" : "s"),
                 game_data.score, game_data.high_score);
    }
    gtk_label_set_text(GTK_LABEL(game_data.status_label), str);
}
