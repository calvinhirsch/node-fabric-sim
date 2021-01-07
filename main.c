#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Config

int fps = 60;
int scale = 16;
float dt;
int monitor = 1;
bool draw_fps = false;

bool draw_circle = false;
bool draw_square = true;
bool draw_grid = false;
bool draw_line = false;
float circle_radius = 3;
int square_size = 16;

float max_dh = 0.3f;
float dh_stretch_force = 6;

float max_h = 7;
float h_stretch_force = 0.7f;

float mouse_force = 0;
float click_mouse_force = -300;
float right_click_mouse_force = 300;

bool incl_sin_drift = true;
float sin_drift_force = 0.3f; // This will reduce the amplitude if it is too low, don't put over 1
float sin_offset = 0;
float sin_offset_roc = 1; // Change in offset per second
float sin_ampl = 10; // I recommend somewhere around max_h, higher if drift force is low
float sin_domain_scale = 0.1f; // Modifies width of waves

float friction_coeff = 0.985f; // Modifies all velocities every frame

// Global vars

int w, h, w_extra, h_extra, width, height, border_x, border_y;
float *fh, *fv, last_mx, last_my;

float clipf(float v, float min, float max) {
    return v > max ? max : (v < min ? min : v);
}

void init_fabric() {
    width = GetMonitorWidth(monitor);
    height = GetMonitorHeight(monitor);
    border_x = 10;
    border_y = clipf(height/12, 10, 9999);
    w = (width - border_x*2) / scale + 1;
    h = (height - border_y*2) / scale + 1;
    w_extra = (int) ((width - border_x*2 - ((float) w - 1) * scale) / 2);
    h_extra = (int) ((height - border_y*2 - ((float) h - 1) * scale) / 2);

    last_mx = (GetMouseX() - w_extra - border_x) / scale;
    last_my = (GetMouseY() - h_extra - border_y) / scale;

    free(fh);
    free(fv);
    fh = calloc(w * h, sizeof(float));
    fv = calloc(w * h, sizeof(float));
}

void apply_cap_force() {
    // Add force for exceeding h max
    int ind;
    int nh;

    for (int i = 1; i < w - 1; i++) {
        for (int j = 1; j < h - 1; j++) {
            ind = j*w+i; // Node index
            nh = fh[ind]; // Node height

            if (nh > max_h) { fv[ind] -= (nh - max_h) * h_stretch_force; }
            if (nh < -max_h) { fv[ind] += (-nh - max_h) * h_stretch_force; }
        }
    }
}

void apply_stretch_force() {
    float nh, dh, force;
    int ind, ind2;
    for (int i = 1; i < w; i++) {
        for (int j = 1; j < h; j++) {
            ind = j*w+i; // Node index
            nh = fh[ind]; // Node height

            ind2 = j*w+i-1;
            dh = fh[ind2] - nh;
            if (dh > max_dh) {
                force = (dh - max_dh) * dh_stretch_force;
                fv[ind] += force;
                fv[ind2] -= force;
            }
            else if (dh < -max_dh) {
                force = (dh + max_dh) * dh_stretch_force;
                fv[ind] += force;
                fv[ind2] -= force;
            }

            ind2 = (j-1)*w+i;
            dh = fh[ind2] - nh;
            if (dh > max_dh) {
                force = (dh - max_dh) * dh_stretch_force;
                fv[ind] += force;
                fv[ind2] -= force;
            }
            else if (dh < -max_dh) {
                force = (dh + max_dh) * dh_stretch_force;
                fv[ind] += force;
                fv[ind2] -= force;
            }
        }
    }
}

void apply_mouse_force() {
    // Apply force based on distance to line from last position to current position
    float m_force = IsMouseButtonDown(0) ? click_mouse_force : (IsMouseButtonDown(1) ? right_click_mouse_force : mouse_force);
    const float dir = m_force < 1 ? -1.0f : 1.0f;
    m_force = fabs(m_force);
    const float mx = (GetMouseX() - w_extra) / scale;
    const float my = (GetMouseY() - h_extra) / scale;
    const float dx = mx - last_mx;
    const float dy = my - last_my;
    const float l2 = dx*dx + dy*dy; // l2 norm^2 of line segment
    const float min_dist = 40 / scale; // Can be adjusted
    const float scale_mult = 1 / (float)scale;
    bool moved = l2 > 0.1;
    float t, proj_x, proj_y, dist_x_last, dist_y_last, dist_x, dist_y, dist, dist_last;
    int ind;

    for (int i = 1; i < w - 1; i++) {
        for (int j = 1; j < h - 1; j++) {
            ind = j * w + i;
            dist_x_last = i - last_mx;
            dist_y_last = j - last_my;

            if (moved) {
                // Project current node onto mouse path
                t = clipf((dist_x_last * dx + dist_y_last * dy) / l2, 0, 1); // Closest t in the segment
                proj_x = last_mx + t * dx; // X position of closest point on segment
                proj_y = last_my + t * dy; // Y position of closest point on segment
                dist_x = proj_x - i;
                dist_y = proj_y - j;
            }
            else {
                t = 1;
                dist_x = i - mx;
                dist_y = j - my;
            }

            dist = sqrt(dist_x * dist_x + dist_y * dist_y) + fh[ind] * dir * scale_mult;
            dist_last = sqrt(dist_x_last * dist_x_last + dist_y_last * dist_y_last)+ fh[ind] * dir * scale_mult;
            if (dist < min_dist) dist = min_dist;
            fv[ind] +=  m_force * clipf(1 / (dist*dist) - 1 / (dist_last*dist_last), 0, 99999) * dir;
        }
    }
    printf("\n");

    last_mx = mx;
    last_my = my;
}

void apply_drift() {
    float sin_val;
    int ind;
    for (int i = 1; i < w - 1; i++) {
        for (int j = 1; j < h - 1; j++) {
            ind = j*w+i;
            sin_val = sin_ampl * (sin(sin_domain_scale*i + sin_offset) + sin(sin_domain_scale*j + sin_offset)) * 0.5;
            fh[ind] += (sin_val - fh[ind]) * dt * sin_drift_force;
        }
    }

    sin_offset += sin_offset_roc * dt;
}

void move() {
    int ind;
    for (int i = 1; i < w - 1; i++) {
        for (int j = 1; j < h - 1; j++) {
            ind = j*w+i;
            fh[ind] += fv[ind] * dt;
            fv[ind] *= friction_coeff;
        }
    }
}


int main() {
    float nh, nv;
    int xpos, ypos;
    Color color;
    char fpss[8];

    InitWindow(500, 500, "Node Fabric");
    width = GetMonitorWidth(monitor);
    height = GetMonitorHeight(monitor);
    SetWindowPosition(0, 0);
    SetWindowSize(width, height);

    SetTargetFPS(fps);
    dt = 1/(float)fps;

    init_fabric();

    // GAME LOOP

    while (!WindowShouldClose()) {
        if (IsWindowResized()) init_fabric();


        // Draw

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 1; i < w-1; i++) {
            for (int j = 1; j < h-1; j++) {
                nh = fh[j*w+i];
                nv = fv[j*w+i];
                xpos = i*scale + w_extra + border_x;
                ypos = (int) ((j-nh)*scale) + h_extra + border_y;
                color = (Color) {
                    .r = (int) clipf(10 + fabs(nv)*scale*2, 0, 255),
                    .g = (int) clipf(180 + 75 * nh / max_h, 0, 255),
                    .b = (int) clipf(180 - 75 * nh / max_h, 0, 255),
                    .a = 255
                };

                if (draw_circle) DrawCircle(xpos, ypos, circle_radius, color);
                if (draw_square) DrawRectangle(xpos-square_size/2, ypos-square_size/2, square_size, square_size, color);

                if (draw_line && i != 1 && j != 1) DrawLine((i-1)*scale+w_extra+border_x, round((j-1-fh[(j-1)*w+i-1])*scale+h_extra+border_y), xpos, ypos, color);
                if (draw_grid) {
                    if (j != 1) DrawLine(xpos, round((j-1-fh[(j-1)*w+i])*scale+h_extra+border_y), xpos, ypos, color);
                    if (i != 1) DrawLine((i-1)*scale+w_extra+border_x, round((j-fh[j*w+i-1])*scale)+h_extra+border_y, xpos, ypos, color);
                }
            }
        }

        if (draw_fps) {
            sprintf(fpss, "%d\n", GetFPS());
            DrawText(fpss, 20, 20, 20, RAYWHITE);
        }

        EndDrawing();


        // Update simulation

        apply_cap_force();
        apply_stretch_force();
        apply_mouse_force();
        if (incl_sin_drift) apply_drift();
        move();
    }


    CloseWindow();

    return 0;
}
