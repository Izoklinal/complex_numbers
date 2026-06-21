#include "raylib.h"
#include <array>
#include <cmath>

Camera cam = {0};

class Quat {
public:
    float scalar;
    float xi;
    float yj;
    float zk;
    Quat(float p_scalar, Vector3 vec3) {
        scalar = p_scalar;
        xi = vec3.x;
        yj = vec3.y;
        zk = vec3.z;
    }
    Quat(float p_scalar, float p_xi, float p_yj, float p_zk) {
        scalar = p_scalar;
        xi = p_xi;
        yj = p_yj;
        zk = p_zk;
    }
    Quat inverse() {
        return Quat(scalar, -xi, -yj, -zk);
    }
    static inline Quat mul(Quat a, Quat b) {
        /*
        w = w1w2 - x1x2 - y1y2 - z1z2
        x = w1x2 + x1w2 + y1z2 - z1y2
        y = w1y2 - x1z2 + y1w2 + z1x2
        z = w1z2 + x1y2 - y1x2 + z1w2
        */
        return Quat(
            a.scalar*b.scalar - a.xi*b.xi - a.yj*b.yj - a.zk*b.zk,
            a.scalar*b.xi + a.xi*b.scalar + a.yj*b.zk - a.zk*b.yj,
            a.scalar*b.yj - a.xi*b.zk + a.yj*b.scalar + a.zk*b.xi,
            a.scalar*b.zk + a.xi*b.yj - a.yj*b.xi + a.zk*b.scalar
        );
    }
};

class Vec3 {
public:
    static inline float magnitude(Vector3 vec3) {
        return sqrtf(powf(vec3.x, 2) + powf(vec3.y, 2) + powf(vec3.z, 2));
    }
    static inline Vector3 normalize(Vector3 a) {
        float mag = magnitude(a);
        return Vector3{ .x = a.x / mag, .y = a.y / mag, .z = a.z / mag };
    }
    static inline Vector3 add_scalar(Vector3 a, float b) {
        return Vector3{ .x = a.x + b, .y = a.y + b, .z = a.z + b };
    }
    static inline Vector3 sub_scalar(Vector3 a, float b) {
        return Vector3{ .x = a.x - b, .y = a.y - b, .z = a.z - b };
    }
    static inline Vector3 mul_scalar(Vector3 a, float b) {
        return Vector3{ .x = a.x * b, .y = a.y * b, .z = a.z * b };
    }
    static inline Vector3 add_vec3(Vector3 a, Vector3 b) {
        return Vector3{ .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z };
    }
    static inline Vector3 sub_vec3(Vector3 a, Vector3 b) {
        return Vector3{ .x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z };
    }
    
    static Vector3 rotate_quaternion(Vector3 vec3, Vector3 axis, float rad) {
        float half_rad = rad * 0.5f;
        float cos = cosf(half_rad);
        float sin = sinf(half_rad);

        axis = normalize(axis);

        Vector3 axis_quat_v = mul_scalar(axis, sin);
        Quat q = Quat(cos, axis_quat_v);
        Quat q_inv = q.inverse();

        Quat v = Quat(0.0f, vec3);

        Quat qv = Quat::mul(q, v);
        Quat qvqinv = Quat::mul(qv, q_inv);
        return Vector3{
            .x = qvqinv.xi,
            .y = qvqinv.yj,
            .z = qvqinv.zk
        };
    }
};

class Cube {
private:
    std::array<Vector3, 8> vs;
    Color col;
public:
    Cube(std::array<Vector3, 8> p_vs, Color p_col) {
        vs = p_vs;
        col = p_col;
    }

    void render_lines() {
        DrawLine3D(vs[0], vs[1], col);
        DrawLine3D(vs[1], vs[2], col);
        DrawLine3D(vs[2], vs[3], col);
        DrawLine3D(vs[3], vs[0], col);

        DrawLine3D(vs[4], vs[5], col);
        DrawLine3D(vs[5], vs[6], col);
        DrawLine3D(vs[6], vs[7], col);
        DrawLine3D(vs[7], vs[4], col);

        DrawLine3D(vs[0], vs[4], col);
        DrawLine3D(vs[1], vs[5], col);
        DrawLine3D(vs[2], vs[6], col);
        DrawLine3D(vs[3], vs[7], col);
    }

    void render_vs() {
        DrawSphere(vs[0], 0.5f, col);
        DrawSphere(vs[1], 0.5f, col);
        DrawSphere(vs[2], 0.5f, col);
        DrawSphere(vs[3], 0.5f, col);
        DrawSphere(vs[4], 0.5f, col);
        DrawSphere(vs[5], 0.5f, col);
        DrawSphere(vs[6], 0.5f, col);
        DrawSphere(vs[7], 0.5f, col);
    }

    void rotate(float rad, Vector3 axis) {
        for (size_t i = 0; i < 8; i++)
        {
            vs[i] = Vec3::rotate_quaternion(vs[i], axis, rad);
        }
    }
};

void init() {
    InitWindow(800, 600, "3D");
    SetTargetFPS(60);
    cam.fovy = 60.0f;
    cam.position = Vector3{
        .x =  0.0f,
        .y =  0.0f,
        .z = 50.0f,
    };
    cam.target = Vector3{
        .x = 0.0f,
        .y = 0.0f,
        .z = 0.0f,
    };
    cam.up = Vector3{
        .x = 0.0f,
        .y = 1.0f,
        .z = 0.0f,
    };
    cam.projection = CAMERA_PERSPECTIVE;
}

int main() {
    init();

    Cube c = Cube(
        {
            Vector3{ .x = -5.0f, .y =  5.0f, .z =  5.0f },
            Vector3{ .x =  5.0f, .y =  5.0f, .z =  5.0f },
            Vector3{ .x =  5.0f, .y =  5.0f, .z = -5.0f },
            Vector3{ .x = -5.0f, .y =  5.0f, .z = -5.0f },

            Vector3{ .x = -5.0f, .y = -5.0f, .z =  5.0f },
            Vector3{ .x =  5.0f, .y = -5.0f, .z =  5.0f },
            Vector3{ .x =  5.0f, .y = -5.0f, .z = -5.0f },
            Vector3{ .x = -5.0f, .y = -5.0f, .z = -5.0f },
        },
        WHITE
    );

    while(!WindowShouldClose()) {
        if (IsKeyDown(KEY_W)) {
            c.rotate(0.01f, { .x = 1.0f, .y = 0.0f, .z = 0.0f });
        }
        
        if (IsKeyDown(KEY_S)) {
            c.rotate(0.01f, { .x = -1.0f, .y = 0.0f, .z = 0.0f });
        }

        if (IsKeyDown(KEY_A)) {
            c.rotate(0.01f, { .x = 0.0f, .y = -1.0f, .z = 0.0f });
        }
        
        if (IsKeyDown(KEY_D)) {
            c.rotate(0.01f, { .x = 0.0f, .y = 1.0f, .z = 0.0f });
        }
        
        if (IsKeyDown(KEY_Q)) {
            c.rotate(0.01f, { .x = 0.0f, .y = 0.0f, .z = 1.0f });
        }

        if (IsKeyDown(KEY_E)) {
            c.rotate(0.01f, { .x = 0.0f, .y = 0.0f, .z = -1.0f });
        }

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(cam);
        {
            c.render_lines();
            // c.render_vs();
        }
        EndMode3D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}