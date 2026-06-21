#include "raylib.h"
#include <random>
#include <array>
#include <iostream>
#include <cmath>
#include <functional>
#include <vector>

Camera2D cam = {0};

const int width = 800;
const int height = 600;
const float rotation = 0.001f;

inline Vector2 vec2sub(Vector2 a, Vector2 b) { 
    return Vector2{ .x = a.x - b.x, .y = a.y - b.y };
}
inline Vector2 vec2add(Vector2 a, Vector2 b) { 
    return Vector2{ .x = a.x + b.x, .y = a.y + b.y };
}
inline Vector2 vec2rotCn(Vector2 a, float rad) {
    /*
    Z = x + yi
    R = cosO + sinOi
    V = Z * R
    V = (x + yi) * (cosO + sinOi)
    V = x*cosO + x*sinOi + yi*cosO + yi*sinOi
    V = x*cosO + x*sinOi + yi*cosO + y*sinO*i^2
    x' = x*cosO + y*sinO*i^2 ('cause i^2 gives just a normal number)
    y' = x*sinOi + yi*cosO
    AND HOLY FUCK THIS SHIT GIVES THE SAME FORMULA FOR ROTATING AS CLASSICAL ROTATION FORMULA
    */
    float sin = sinf(rad);
    float cos = cosf(rad);

    return Vector2{
        .x = a.x*cos - a.y*sin,
        .y = a.x*sin + a.y*cos
    };
}

class Rect {
private:
    std::array<Vector2, 4> points; 
    Vector2 center;
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
    Rect(const std::array<Vector2, 4>& p_points) : points(p_points) {
        setCenter();
    }

    static Rect createSectRandRect(int screenWidth, int screenHeight, int padding = 0) {
        int halfWidth = screenWidth * 0.5f;
        int halfHeight = screenHeight * 0.5f;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distribX(0.0f, halfWidth);
        std::uniform_real_distribution<float> distribY(0.0f, halfHeight);

        return Rect({
            Vector2{
                .x = fminf(distribX(gen) + padding, halfWidth - 1),
                .y = fminf(distribY(gen) + padding, halfHeight - 1)
            },
            Vector2{
                .x = fmaxf(halfWidth + distribX(gen) - padding, halfWidth + 1),
                .y = fminf(distribY(gen) + padding, halfHeight - 1)
            },
            Vector2{
                .x = fmaxf(halfWidth + distribX(gen) - padding, halfWidth + 1),
                .y = fmaxf(halfHeight + distribY(gen) - padding, halfHeight + 1)
            },
            Vector2{
                .x = fminf(distribX(gen) + padding, halfWidth - 1),
                .y = fmaxf(halfHeight + distribY(gen) - padding, halfHeight + 1)
            }
        });
    }

    void draw() {
        DrawTriangle(points[0], points[2], points[1], WHITE);
        DrawTriangle(points[0], points[3], points[2], WHITE);
    }

    void drawCenter() {
        DrawCircleV(center, 5.0f, GRAY);
    }

    void drawPoints() {
        const int font = 15;

        DrawText("1", points[0].x, points[0].y, font, WHITE);
        DrawText("2", points[1].x, points[1].y, font, WHITE);
        DrawText("3", points[2].x, points[2].y, font, WHITE);
        DrawText("4", points[3].x, points[3].y, font, WHITE);
    }

    void drawLines() {
        DrawLineV(points[0], points[1], WHITE);
        DrawLineV(points[1], points[2], WHITE);
        DrawLineV(points[2], points[3], WHITE);
        DrawLineV(points[3], points[0], WHITE);
    }

    void rotate(float rad) {
       std::array<Vector2, 4> centeredPoints = points;
       for (size_t i = 0; i < 4; i++)
       {
            // 1. Centralize the point
            centeredPoints[i] = vec2sub(points[i], center);
            // 2. Rotate the point
            centeredPoints[i] = vec2rotCn(centeredPoints[i], rad);
            // 3. Return point to its actual position
            points[i] = vec2add(centeredPoints[i], center);
        }
    }
};
struct RenderTask{
    int id;
    std::function<void()> task;
};

class Renderer {
private:
    std::vector<RenderTask> tasks;
    public:
    bool hasTask(int id) {
        for (const auto& task : tasks) {
            if (task.id == id) {
                return true;
            }
        }
        return false;
    }
    void addTask(RenderTask task) {
        if (!hasTask(task.id)) {
            tasks.push_back(task);
        }
    }
    void removeTask(int id) {
        for (auto it = tasks.begin(); it != tasks.end(); ++it) {
            if (it->id == id) {
                tasks.erase(it);
                return;
            }
        }
    }
    void execute() {
        for (auto& task : tasks) {
            task.task();
        }
    }
};


int main() {
    InitWindow(width, height, "Ahoj!");
    cam.zoom = 1.0f;

    int padding = 50;

    Rect rect = Rect::createSectRandRect(width, height, padding);

    Renderer renderer;
    renderer.addTask(RenderTask{
        .id = 1,
        .task = [&rect]() { rect.draw(); }
    });

    while(!WindowShouldClose()) {
        ClearBackground(BLACK);

        if (IsKeyPressed(KEY_R)) {
            rect = Rect::createSectRandRect(width, height, padding);
        }

        if (IsKeyDown(KEY_D)) {
            rect.rotate(rotation);
        }

        if (IsKeyDown(KEY_A)) {
            rect.rotate(-rotation);
        }

        if (IsKeyPressed(KEY_ONE)) {
            if (renderer.hasTask(1)) {
                renderer.removeTask(1);
            } else {
                renderer.addTask(RenderTask{
                    .id = 1,
                    .task = [&rect]() { rect.draw(); }
                });
            }
        }

        if (IsKeyPressed(KEY_TWO)) {
            if (renderer.hasTask(2)) {
                renderer.removeTask(2);
            } else {
                renderer.addTask(RenderTask{
                    .id = 2,
                    .task = [&rect]() { rect.drawCenter(); }
                });
            }
        }
        
        if (IsKeyPressed(KEY_THREE)) {
            if (renderer.hasTask(3)) {
                renderer.removeTask(3);
            } else {
                renderer.addTask(RenderTask{
                    .id = 3,
                    .task = [&rect]() { rect.drawPoints(); }
                });
            }
        }

        if (IsKeyPressed(KEY_FOUR)) {
            if (renderer.hasTask(4)) {
                renderer.removeTask(4);
            } else {
                renderer.addTask(RenderTask{
                    .id = 4,
                    .task = [&rect]() { rect.drawLines(); }
                });
            }
        }
        
        BeginDrawing();
        BeginMode2D(cam);
        {
            renderer.execute();
        }
        EndMode2D();
        EndDrawing();
    }

    return 0;
}