#pragma once
#include <Geometry.h>

class AudioCubesPattern final : public Pattern
{

    class CubeObject
    {
      public:
        float focal = 30;                                                    // Focal of the camera
        float speed = 100;                                                   // Focal of the camera
        float Angx = 20.0, AngxSpeed = 0.05;                                 // rotation (angle+speed) around X-axis
        float Angy = 10.0, AngySpeed = 0.05;                                 // rotation (angle+speed) around Y-axis
        float Ox = ((MATRIX_WIDTH - 1) / 2), Oy = ((MATRIX_HEIGHT - 1) / 2); // position (x,y) of the frame center
        int zCamera = 110; // distance from cube to the eye of the camera

        // Local vertices
        Vertex local[8];
        // Camera aligned vertices
        Vertex aligned[8];
        // On-screen projected vertices
        Point screen[8];
        // Faces
        SquareFace face[6];
        // Edges
        EdgePoint edge[12];
        int nbEdges;
        // ModelView matrix
        float m00, m01, m02, m10, m11, m12, m20, m21, m22;

        // individualy defined now
        byte hue = 0;
        byte bin = 0;
        int step = 0;

        // constructs the cube
        void make(const int w)
        {
            nbEdges = 0;

            local[0].set(-w, w, w);
            local[1].set(w, w, w);
            local[2].set(w, -w, w);
            local[3].set(-w, -w, w);
            local[4].set(-w, w, -w);
            local[5].set(w, w, -w);
            local[6].set(w, -w, -w);
            local[7].set(-w, -w, -w);

            face[0].set(1, 0, 3, 2);
            face[1].set(0, 4, 7, 3);
            face[2].set(4, 0, 1, 5);
            face[3].set(4, 5, 6, 7);
            face[4].set(1, 2, 6, 5);
            face[5].set(2, 3, 7, 6);

            int f, i;
            for (f = 0; f < 6; f++)
            {
                for (i = 0; i < face[f].length; i++)
                {
                    face[f].ed[i] = this->findEdge(face[f].sommets[i], face[f].sommets[i ? i - 1 : face[f].length - 1]);
                }
            }
        }

        // finds edges from faces
        int findEdge(const int a, const int b)
        {
            int i;
            for (i = 0; i < nbEdges; i++)
                if ((edge[i].x == a && edge[i].y == b) || (edge[i].x == b && edge[i].y == a))
                    return i;
            edge[nbEdges++].set(a, b);
            return i;
        }

        void rotate(const float angx, const float angy)
        {
            int i;
            const float cx = cos(angx);
            const float sx = sin(angx);
            const float cy = cos(angy);
            const float sy = sin(angy);

            m00 = cy;
            m01 = 0;
            m02 = -sy;
            m10 = sx * sy;
            m11 = cx;
            m12 = sx * cy;
            m20 = cx * sy;
            m21 = -sx;
            m22 = cx * cy;

            for (i = 0; i < 8; i++)
            {
                aligned[i].x = m00 * local[i].x + m01 * local[i].y + m02 * local[i].z;
                aligned[i].y = m10 * local[i].x + m11 * local[i].y + m12 * local[i].z;
                aligned[i].z = m20 * local[i].x + m21 * local[i].y + m22 * local[i].z + zCamera;

                screen[i].x = floor((Ox + focal * aligned[i].x / aligned[i].z));
                screen[i].y = floor((Oy - focal * aligned[i].y / aligned[i].z));
            }

            for (i = 0; i < 12; i++)
                edge[i].visible = false;

            Point *pa, *pb, *pc;
            for (i = 0; i < 6; i++)
            {
                pa = screen + face[i].sommets[0];
                pb = screen + face[i].sommets[1];
                pc = screen + face[i].sommets[2];

                const boolean back = ((pb->x - pa->x) * (pc->y - pa->y) - (pb->y - pa->y) * (pc->x - pa->x)) < 0;
                if (!back)
                {
                    int j;
                    for (j = 0; j < 4; j++)
                    {
                        edge[face[i].ed[j]].visible = true;
                    }
                }
            }
        }
    };

    uint8_t cubeCount = 0;
    std::vector<CubeObject> cube;

  public:
    static constexpr auto ID = "AudioCubes";

    explicit AudioCubesPattern(MatrixLeds &leds, MatrixNoise &noise, AudioContext &audio)
        : Pattern(ID, leds, noise, audio)
    {
    }

    void start() override
    {
        // cubeCount = random8(1, 4);
        cubeCount = 3;
        cube.resize(cubeCount);
        for (int c = 0; c < cubeCount; c++)
        {
            cube[c].make(random8(60, 80));
            cube[c].Angx = random8(10, 21);
            cube[c].Angy = random8(10, 21);
            cube[c].hue = random8();
            cube[c].speed = random8(100, 121);
            cube[c].bin = random8(0, MATRIX_WIDTH);
        }
    }

    void render() override
    {
        for (int c = 0; c < cubeCount; c++)
        {
            if (audio.isBeat)
            {
                cube[c].hue += audio.energy8Scaled >> 4;
            }

            cube[c].zCamera = 255.0f - (audio.peaks8[cube[c].bin] >> 1);
            cube[c].AngxSpeed = beatsin8(3, 1, 10) / 100.0f;
            cube[c].AngySpeed = beatcos8(5, 1, 10) / 100.0f;
            cube[c].Angx += cube[c].AngxSpeed;
            cube[c].Angy += cube[c].AngySpeed;

            if (cube[c].Angx >= TWO_PI)
                cube[c].Angx -= TWO_PI;
            if (cube[c].Angy >= TWO_PI)
                cube[c].Angy -= TWO_PI;

            cube[c].rotate(cube[c].Angx, cube[c].Angy);

            int i;

            EdgePoint *e;
            for (i = 0; i < 12; i++)
            {
                e = cube[c].edge + i;
                if (!e->visible)
                {
                    drawLine(
                        cube[c].screen[e->x].x,
                        cube[c].screen[e->x].y,
                        cube[c].screen[e->y].x,
                        cube[c].screen[e->y].y,
                        ColorFromPalette(RainbowColors_p, cube[c].hue, 255));
                }
            }

            for (i = 0; i < 12; i++)
            {
                e = cube[c].edge + i;
                if (e->visible)
                {
                    drawLine(
                        cube[c].screen[e->x].x,
                        cube[c].screen[e->x].y,
                        cube[c].screen[e->y].x,
                        cube[c].screen[e->y].y,
                        ColorFromPalette(RainbowColors_p, cube[c].hue, 255));
                }
            }

            cube[c].step++;
            if (cube[c].step == 8)
            {
                cube[c].step = 0;
                cube[c].hue++;
            }
        }
    }
};
