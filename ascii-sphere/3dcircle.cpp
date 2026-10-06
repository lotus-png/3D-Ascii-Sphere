#include <cmath>
#include <cstdio>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>

// M_PI is not standard C++, so define our own.
const float PI = 3.14159265358979f;

// Terminal screen size. Tweak to fit your terminal window.
const int WIDTH = 120;
const int HEIGHT = 44;

// Sphere radius (in 3D units) and how far away the camera is.
const float RADIUS = 22.0f;
const float DISTANCE_FROM_CAM = 60.0f;
const float K1 = 40.0f; // projection scale factor

// How finely we sample the surface. Smaller step = denser/smoother sphere.
const float THETA_STEP = 0.07f; // around the vertical axis (0 to 2*PI)
const float PHI_STEP   = 0.02f; // pole to pole (0 to PI)

// Shading ramp from darkest to brightest. Index chosen by how directly
// each surface point faces the light.
const char SHADE_CHARS[] = ".,-~:;=!*#$@";
const int SHADE_COUNT = sizeof(SHADE_CHARS) - 1; // exclude the null terminator

// Light comes from the upper-left-front, normalized.
const float LIGHT_X = -0.5f, LIGHT_Y = 0.6f, LIGHT_Z = -0.7f;

float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;

void rotate(float x, float y, float z, float& outX, float& outY, float& outZ)
{
    // Rotate around X
    float y1 = y * cosf(angleX) - z * sinf(angleX);
    float z1 = y * sinf(angleX) + z * cosf(angleX);

    // Rotate around Y
    float x2 = x * cosf(angleY) + z1 * sinf(angleY);
    float z2 = -x * sinf(angleY) + z1 * cosf(angleY);

    // Rotate around Z
    float x3 = x2 * cosf(angleZ) - y1 * sinf(angleZ);
    float y3 = x2 * sinf(angleZ) + y1 * cosf(angleZ);

    outX = x3;
    outY = y3;
    outZ = z2;
}

void plot(float x, float y, float z, float nx, float ny, float nz,
        std::vector<char>& buffer, std::vector<float>& zbuffer)
{
    // Rotate both the surface point and its normal (normals rotate the
    // same way as positions since we're only doing rigid rotation).
    float rx, ry, rz, rnx, rny, rnz;
    rotate(x, y, z, rx, ry, rz);
    rotate(nx, ny, nz, rnx, rny, rnz);

    float camZ = rz + DISTANCE_FROM_CAM;
    if (camZ <= 0.0f) return; // behind the camera

    float invZ = 1.0f / camZ;
    int screenX = static_cast<int>(WIDTH / 2 + K1 * invZ * rx * 2.0f); // *2 corrects character aspect ratio
    int screenY = static_cast<int>(HEIGHT / 2 - K1 * invZ * ry);

    if (screenX < 0 || screenX >= WIDTH || screenY < 0 || screenY >= HEIGHT) return;

    int idx = screenY * WIDTH + screenX;
    if (invZ <= zbuffer[idx]) return; // something closer already drawn here
    // Lambertian shading: how directly does this point's normal face the light?
    float luminance = rnx * LIGHT_X + rny * LIGHT_Y + rnz * LIGHT_Z;
    if (luminance < 0.0f) luminance = 0.0f; // facing away from the light

    int shadeIdx = static_cast<int>(luminance * (SHADE_COUNT - 1));
    if (shadeIdx < 0) shadeIdx = 0;
    if (shadeIdx >= SHADE_COUNT) shadeIdx = SHADE_COUNT - 1;

    zbuffer[idx] = invZ;
    buffer[idx] = SHADE_CHARS[shadeIdx];
}

int main()
{
    std::vector<char> buffer(WIDTH * HEIGHT);
    std::vector<float> zbuffer(WIDTH * HEIGHT);

    printf("\x1b[2J\x1b[?25l"); // clear once, hide cursor

    while (true)
    {
        std::fill(buffer.begin(), buffer.end(), ' ');
        std::fill(zbuffer.begin(), zbuffer.end(), 0.0f);

        for (float theta = 0.0f; theta < 2.0f * PI; theta += THETA_STEP)
        {
            float cosT = cosf(theta), sinT = sinf(theta);
            for (float phi = 0.0f; phi < PI; phi += PHI_STEP)
            {
                float cosP = cosf(phi), sinP = sinf(phi);

                // Standard spherical -> Cartesian coordinates.
                float nx = sinP * cosT;
                float ny = cosP;
                float nz = sinP * sinT;

                float x = RADIUS * nx;
                float y = RADIUS * ny;
                float z = RADIUS * nz;

                // For a sphere the outward normal is just the point direction.
                plot(x, y, z, nx, ny, nz, buffer, zbuffer);
            }
        }

        printf("\x1b[H"); // cursor to top-left, no full clear -> no flicker
        std::string frame;
        frame.reserve(WIDTH * HEIGHT + HEIGHT);
        for (int y = 0; y < HEIGHT; ++y)
        {
            frame.append(&buffer[y * WIDTH], WIDTH);
            frame.push_back('\n');
        }
        fwrite(frame.data(), 1, frame.size(), stdout);
        fflush(stdout);

        angleX += 0.03f;
        angleY += 0.05f;
        angleZ += 0.01f;

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    return 0;
}