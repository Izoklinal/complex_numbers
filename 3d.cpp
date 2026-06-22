#include "raylib.h"
#include <array>
#include <iostream>
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
        Q1*Q1 = w1w2  + w1x2i  + w1y2j  + w1z2k  +
              + x1iw2 + x1ix2i + x1iy2j + x1iz2k +
              + y1jw2 + y1jx2i + y1jy2j + y1jz2k +
              + z1kw2 + z1kx2i + z1ky2j + z1kz2k
        
        Q1*Q2 = w1w2  + w1x2i + w1y2j + w1z2k +
              + x1w2i - x1x2  + x1y2k - x1z2j +
              + y1w2j - y1x2k - y1y2  + y1z2i +
              + z1w2k + z1x2j - z1y2i - z1z2
        
        Q1*Q2 = (w1w2 - x1x2 - y1y2 - z1z2)  +
              + (w1x2 + x1w2 + y1z2 0 z1y2)i +
              + (w1y2 - x1z2 + y1w2 + z1x2)j +
              + (w1z2 + x1y2 - y1x2 + z1w2)k

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
    static inline Vector3 div_scalar(Vector3 a, float b) {
        return Vector3{ .x = a.x / b, .y = a.y / b, .z = a.z / b };
    }
    static inline Vector3 add_vec3(Vector3 a, Vector3 b) {
        return Vector3{ .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z };
    }
    static inline Vector3 sub_vec3(Vector3 a, Vector3 b) {
        return Vector3{ .x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z };
    }
    static inline float dot_prod(Vector3 a, Vector3 b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
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
        qv = Quat::mul(qv, q_inv);
        return Vector3{
            .x = qv.xi,
            .y = qv.yj,
            .z = qv.zk
        };
    }
};

class Cube {
private:
    std::array<Vector3, 8> vs;
    std::array<Vector3, 6> sides;
    std::array<Vector3, 6> normales;
    const float MIN_DOT_PROD = -0.1f;
    Vector3 center;
    Color col;

    void init_center() {
        center = Vector3(0.0f, 0.0f, 0.0f);
        for (size_t i = 0; i < 8; i++)
        {
            center = Vec3::add_vec3(center, vs[i]);
        }
        center = Vec3::div_scalar(center, 8.0f);
    }
    void init_sides() {
        sides[0] = {
            .x = (vs[0].x + vs[1].x + vs[2].x + vs[3].x) / 4.0f,
            .y = (vs[0].y + vs[1].y + vs[2].y + vs[3].y) / 4.0f,
            .z = (vs[0].z + vs[1].z + vs[2].z + vs[3].z) / 4.0f,
        };

        sides[1] = {
            .x = (vs[4].x + vs[5].x + vs[6].x + vs[7].x) / 4.0f,
            .y = (vs[4].y + vs[5].y + vs[6].y + vs[7].y) / 4.0f,
            .z = (vs[4].z + vs[5].z + vs[6].z + vs[7].z) / 4.0f,
        };

        sides[2] = {
            .x = (vs[0].x + vs[3].x + vs[4].x + vs[7].x) / 4.0f,
            .y = (vs[0].y + vs[3].y + vs[4].y + vs[7].y) / 4.0f,
            .z = (vs[0].z + vs[3].z + vs[4].z + vs[7].z) / 4.0f,
        };

        sides[3] = {
            .x = (vs[1].x + vs[2].x + vs[5].x + vs[6].x) / 4.0f,
            .y = (vs[1].y + vs[2].y + vs[5].y + vs[6].y) / 4.0f,
            .z = (vs[1].z + vs[2].z + vs[5].z + vs[6].z) / 4.0f,
        };

        sides[4] = {
            .x = (vs[0].x + vs[1].x + vs[5].x + vs[4].x) / 4.0f,
            .y = (vs[0].y + vs[1].y + vs[5].y + vs[4].y) / 4.0f,
            .z = (vs[0].z + vs[1].z + vs[5].z + vs[4].z) / 4.0f,
        };

        sides[5] = {
            .x = (vs[3].x + vs[2].x + vs[7].x + vs[6].x) / 4.0f,
            .y = (vs[3].y + vs[2].y + vs[7].y + vs[6].y) / 4.0f,
            .z = (vs[3].z + vs[2].z + vs[7].z + vs[6].z) / 4.0f,
        };
    }
    void init_normales() {
        for (size_t i = 0; i < 6; i++)
        {
            normales[i] = Vec3::normalize(Vec3::sub_vec3(center, sides[i]));
        }
    }
public:
    Cube(std::array<Vector3, 8> p_vs, Color p_col) {
        vs = p_vs;
        col = p_col;
        
        init_center();
        init_sides();
        init_normales();
    }

    void render_center() {
        DrawSphere(center, 0.1f, col);
    }

    void render_sides() {
        for (size_t i = 0; i < 2; i++)
        {
            DrawSphere(sides[i], 0.1f, col);
            Vector3 dist_btw = Vec3::sub_vec3(sides[i], center);
            dist_btw = Vec3::add_vec3(sides[i], dist_btw);
            DrawLine3D(sides[i], dist_btw, RED);
        }
        for (size_t i = 2; i < 4; i++)
        {
            DrawSphere(sides[i], 0.1f, col);
            Vector3 dist_btw = Vec3::sub_vec3(sides[i], center);
            dist_btw = Vec3::add_vec3(sides[i], dist_btw);
            DrawLine3D(sides[i], dist_btw, GREEN);
        }
        for (size_t i = 4; i < 6; i++)
        {
            DrawSphere(sides[i], 0.1f, col);
            Vector3 dist_btw = Vec3::sub_vec3(sides[i], center);
            dist_btw = Vec3::add_vec3(sides[i], dist_btw);
            DrawLine3D(sides[i], dist_btw, BLUE);
        }
    }

    void render_lines() {
        // up
        if (Vec3::dot_prod(Vec3::normalize(cam.position), normales[0]) < MIN_DOT_PROD) {
            DrawLine3D(vs[0], vs[1], col);
            DrawLine3D(vs[1], vs[2], col);
            DrawLine3D(vs[2], vs[3], col);
            DrawLine3D(vs[3], vs[0], col);
        } 

        // down
        if (Vec3::dot_prod(Vec3::normalize(cam.position), normales[1]) < MIN_DOT_PROD) {
            DrawLine3D(vs[4], vs[5], col);
            DrawLine3D(vs[5], vs[6], col);
            DrawLine3D(vs[6], vs[7], col);
            DrawLine3D(vs[7], vs[4], col);
        }

        // front
        if (Vec3::dot_prod(Vec3::normalize(cam.position), normales[2]) < MIN_DOT_PROD) {
            DrawLine3D(vs[0], vs[3], col);
            DrawLine3D(vs[3], vs[7], col);
            DrawLine3D(vs[7], vs[4], col);
            DrawLine3D(vs[4], vs[0], col);
        }

        // back
        if (Vec3::dot_prod(Vec3::normalize(cam.position), normales[3]) < MIN_DOT_PROD) {
            DrawLine3D(vs[1], vs[2], col);
            DrawLine3D(vs[2], vs[6], col);
            DrawLine3D(vs[6], vs[5], col);
            DrawLine3D(vs[5], vs[1], col);
        }

        // left
        if (Vec3::dot_prod(Vec3::normalize(cam.position), normales[4]) < MIN_DOT_PROD) {
            DrawLine3D(vs[0], vs[1], col);
            DrawLine3D(vs[1], vs[5], col);
            DrawLine3D(vs[5], vs[4], col);
            DrawLine3D(vs[4], vs[0], col);
        }

        // right
        if (Vec3::dot_prod(Vec3::normalize(cam.position), normales[5]) < MIN_DOT_PROD) {
            DrawLine3D(vs[3], vs[2], col);
            DrawLine3D(vs[2], vs[6], col);
            DrawLine3D(vs[6], vs[7], col);
            DrawLine3D(vs[7], vs[3], col);
        }
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
            vs[i] = Vec3::sub_vec3(vs[i], center);
            vs[i] = Vec3::rotate_quaternion(vs[i], axis, rad);
            vs[i] = Vec3::add_vec3(vs[i], center);
        }
        for (size_t i = 0; i < 6; i++)
        {
            sides[i] = Vec3::sub_vec3(sides[i], center);
            sides[i] = Vec3::rotate_quaternion(sides[i], axis, rad);
            sides[i] = Vec3::add_vec3(sides[i], center);
        }
        init_normales();
    }
    
    void move(Vector3 move) {
        for (size_t i = 0; i < 8; i++)
        {
            vs[i] = Vec3::add_vec3(vs[i], move);
        }
        for (size_t i = 0; i < 6; i++)
        {
            sides[i] = Vec3::add_vec3(sides[i], move);
        }
        init_center();
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

        if (IsKeyDown(KEY_O)) {
            c.move({ .x = 0.0f, .y = 0.1f, .z = 0.0f });
        }

        if (IsKeyDown(KEY_L)) {
            c.move({ .x = 0.0f, .y = -0.1f, .z = 0.0f });
        }

        if (IsKeyDown(KEY_K)) {
            c.move({ .x = -0.1f, .y = 0.0f, .z = 0.0f });
        }

        if (IsKeyDown(KEY_SEMICOLON)) {
            c.move({ .x = 0.1f, .y = 0.0f, .z = 0.0f });
        }

        if (IsKeyDown(KEY_I)) {
            c.move({ .x = 0.0f, .y = 0.0f, .z = 0.1f });
        }

        if (IsKeyDown(KEY_P)) {
            c.move({ .x = 0.0f, .y = 0.0f, .z = -0.1f });
        }

        if (IsKeyDown(KEY_EQUAL)) {
            cam.fovy += 0.1f;
        } 

        if (IsKeyDown(KEY_MINUS)) {
            cam.fovy -= 0.1f;
        } 

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(cam);
        {
            c.render_lines();
            c.render_center();
            c.render_sides();
        }
        EndMode3D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}