#include "raylib.h"
#include <random>
#include <array>
#include <iostream>

Camera2D cam = {0};

class Rect {
private:
    std::array<Vector2, 4> points;
    void setCenter() {
        center = findCenter();
    }
    Vector2 findCenter() {
        return {
            .x = (points[0].x + points[1].x + points[2].x + points[3].x) / 4.0f,
            .y = (points[0].y + points[1].y + points[2].y + points[3].y) / 4.0f,
        };
    }
public:
    Vector2 center;
    Rect(const std::array<Vector2, 4>& p_points) : points(p_points) {
        setCenter();
    }

    void draw() {
        DrawTriangle(points[0], points[1], points[2], WHITE);
        DrawTriangle(points[0], points[2], points[3], WHITE);
    }

    void drawCenter() {
        DrawCircleV(center, 5.0f, GRAY);
    }

    void drawPoints() {
        DrawCircleV(points[0], 2.5f, GREEN);
        DrawCircleV(points[1], 2.5f, BLUE);
        DrawCircleV(points[2], 2.5f, YELLOW);
        DrawCircleV(points[3], 2.5f, RED);
    }
    
    static Rect createRandom(std::uniform_real_distribution<float>& distrib, std::mt19937& gen) {
        std::array<Vector2, 4> px = {
            Vector2{distrib(gen), distrib(gen)},
            Vector2{distrib(gen), distrib(gen)},
            Vector2{distrib(gen), distrib(gen)},
            Vector2{distrib(gen), distrib(gen)}
        };

        for (size_t i = 0; i < 4; i++)
        {
            for (size_t j = 0; j < 4; j++)
            {
                if (i == j) continue;

                if (px[i].x < px[j].x) {
                    Vector2 temp = px[i];
                    px[i] = px[j];
                    px[j] = temp;
                }
            }
        }
        
        Vector2 center;
        center.x = (px[0].x + px[1].x + px[2].x + px[3].x) / 4.0f;
        center.y = (px[0].y + px[1].y + px[2].y + px[3].y) / 4.0f;

        Vector2 ul = {0};
        for (size_t i = 0; i < 4; i++)
        {
            ul = px[i];
            if (ul.y < center.y) break;
        }
        
        Vector2 ur = {0};
        for (int i = 3; i >= 0; i--)
        {
            ur = px[i];
            if (ur.y < center.y) break;
        }

        Vector2 dl = {0};
        for (size_t i = 0; i < 4; i++)
        {
            dl = px[i];
            if (dl.y > center.y) break;
        }
        
        Vector2 dr = {0};
        for (int i = 3; i >= 0; i--)
        {
            dr = px[i];
            if (dr.y > center.y) break;
        }

        std::array<Vector2, 4> sorted = {ul, ur, dl, dr};
        return Rect(sorted);
    }
};

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(100.0f, 500.0f);
    
    InitWindow(800, 600, "Ahoj!");
    cam.zoom = 1.0f;

    Rect rect = Rect::createRandom(distrib, gen);

    while(!WindowShouldClose()) {
        ClearBackground(BLACK);

        if (IsKeyPressed(KEY_R)) {
            rect = Rect::createRandom(distrib, gen);
        }
        
        BeginDrawing();
        BeginMode2D(cam);
        {
            rect.draw();
            rect.drawCenter();
            rect.drawPoints();
        }
        EndMode2D();
        EndDrawing();
    }

    return 0;
}