#include <iostream>
#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

const int screenWidth = 1280;
const int screenHeight = 768;

Vector2 ball_pos = Vector2Zero();
float ball_speed_x = 0.0f;
float ball_speed_y = 0.0f;

float y_progress = 0;
float x_progress = 0;

bool falling = false;

int ball_color = 0;
Color ball_colors[] = { WHITE , RED, BLUE, PINK, GREEN, ORANGE, PURPLE, YELLOW };

bool particle_enabled = false;
time_point<steady_clock> particle_start_time = steady_clock::now();

// Modifiable values that does not affect code  (Only the simulation)
    int ball_side_force = 10;
    int ball_jump_force = -10;
    int ball_radius = 8;

    int gravity = 10;

    bool physics = false;

    float energy_lost_per_bounce = -0.85f;
    float ground_friction = 0.95f;
    float air_friction = 0.999f;

    int debug_line_length = 5;

    int particle_lifetime = 3;

    float Mouse_range = 15;

void color_change();

//A single particle
class particle {
public:
    Vector2 velocity;
    Vector2 pos;
    int size;
    
    particle() {
        velocity = Vector2Zero();
        pos = Vector2Zero();
        size = 1;
    }
};

//particles array is reused for every spawn, no new array or set of particle classes is made
particle particles[15];
int particle_amount = sizeof(particles)/sizeof(*particles);

int main() {
    cout << "Starting.." << endl;

    //start ball at middle top
    ball_pos.x = static_cast<float>(screenWidth) / 2;
    ball_pos.y = ball_radius;

    //initiate window
    InitWindow(screenWidth, screenHeight, "ball physics");
    SetTargetFPS(60);

    //main loop
    while (WindowShouldClose() == false) {

        //SPACE toggles physics
        if (IsKeyPressed(32)) physics = !physics;

        //E spawns a new set of particles
        if (IsKeyPressed(69)) {
            srand(time(0));

            //assign random velocity and size to each particle class
            for (int i = 0; i < particle_amount; i++) {
                particles[i].pos = ball_pos;
                particles[i].velocity.x = (rand() % 10) - 5;
                particles[i].velocity.y = (rand() % 10) - 5;
                particles[i].size = (rand() % 5) + 1;
            }
            
            particle_start_time = steady_clock::now();
            particle_enabled = true;
        }

        if (physics) {

            //gravity and vertical bounds
            if ((ball_pos.y + ball_radius) < screenHeight) {
                if ((ball_pos.y - ball_radius) < 0) {
                    color_change();
                    ball_speed_y *= energy_lost_per_bounce;
                    ball_pos.y = ball_radius;
                }

                if (!falling) {
                    falling = true;
                }

                //apply gravity
                ball_speed_y += 1.0f / 6;
            }
            else { 
                //clamp vertical speed to avoid infinite bounces
                if (ball_speed_y < 2.0f) { ball_speed_y = 0; }
                else {
                    color_change();
                    ball_speed_y *= energy_lost_per_bounce;
                    ball_pos.y = screenHeight - ball_radius;
                    falling = false;
                }
            }

            //jump
            if (IsKeyPressed(87)) {
                ball_speed_y += ball_jump_force;
            }

            ball_pos.y += ball_speed_y;

            //push ball away from mouse
            if (Vector2Distance(ball_pos, GetMousePosition()) <= ball_radius + Mouse_range) {
                if (ball_pos.x - GetMouseX() < 0) {
                    ball_speed_x -= ball_side_force;
                }
                else {
                    ball_speed_x += ball_side_force;
                }
            }

            //bounce ball back from walls
            if (ball_pos.x - ball_radius < 0) { 
                color_change();
                ball_speed_x *= energy_lost_per_bounce; 
                ball_pos.x = ball_radius;
            }
            else if (ball_pos.x + ball_radius > screenWidth) {
                color_change();
                ball_speed_x *= energy_lost_per_bounce;
                ball_pos.x = screenWidth - ball_radius;
            }

            //apply appropriate friction on horizontal movement
            if (!falling) ball_speed_x *= ground_friction;
            else ball_speed_x *= air_friction;

            ball_pos.x += ball_speed_x;
        }

        //assign values for GUI bars
        x_progress = Remap(fabs(ball_speed_x), 0, 20.0f, 0, 1.0f);
        y_progress = Remap(fabs(ball_speed_y), 0, 20.0f, 0, 1.0f);

        //time since particles spawned
        auto elasped_time = duration_cast<milliseconds>(steady_clock::now() - particle_start_time);

        //render shapes
        BeginDrawing();
        ClearBackground(BLACK);

        //GUI
        GuiProgressBar(Rectangle{ 28, 4, 120, 16 }, "X", NULL, &x_progress, 0, 1);
        GuiProgressBar(Rectangle{ 28, 28, 120, 16 }, "Y", NULL, &y_progress, 0, 1);

        //Close window button
        if (GuiButton(Rectangle{ screenWidth - 28, 4, 24, 24 }, "X")) {
            EndDrawing();
            CloseWindow();
            return 0;
        }

        //Main Ball
        DrawCircle(ball_pos.x, ball_pos.y, ball_radius, ball_colors[ball_color]);
        DrawLine(ball_pos.x, ball_pos.y, ball_pos.x + (ball_speed_x * debug_line_length), ball_pos.y + (ball_speed_y * debug_line_length), RED );
        
        //Cursor range visualization
        DrawCircleV(GetMousePosition(), Mouse_range, Color{150,150,150, 100});

        //Particles
        if (particle_enabled && elasped_time < seconds(particle_lifetime)) {
            for (int i = 0; i < particle_amount; i++) {
                particles[i].pos.x += particles[i].velocity.x;
                particles[i].pos.y += particles[i].velocity.y;
                DrawCircle(particles[i].pos.x, particles[i].pos.y, particles[i].size * (1 - ((elasped_time.count() / 1000.0f) / particle_lifetime)), ball_colors[ball_color]);
            }
        }
        else if (particle_enabled) {
            particle_enabled = false;
            cout << particle_enabled << endl;
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

//loop through colors array
void color_change() {
    if (ball_color == 7) {
        ball_color = 0;
    }
    else {
        ball_color++;
    }
}