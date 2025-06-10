#pragma once

class Cell
{
  public:
    byte alive : 1;
    byte prev : 1;
    byte hue : 6;
    byte brightness;
};

class LifePattern final : public Pattern
{
    Cell world[MATRIX_WIDTH][MATRIX_HEIGHT];
    unsigned int density = 50;
    uint16_t generation = 0;

    void randomFillWorld()
    {
        for (int i = 0; i < MATRIX_WIDTH; i++)
        {
            for (int j = 0; j < MATRIX_HEIGHT; j++)
            {
                if (random(100) < density)
                {
                    world[i][j].alive = 1;
                    world[i][j].brightness = 255;
                }
                else
                {
                    world[i][j].alive = 0;
                    world[i][j].brightness = 0;
                }
                world[i][j].prev = world[i][j].alive;
                world[i][j].hue = 0;
            }
        }
    }

    int neighbours(int x, int y)
    {
        return (world[(x + 1) % MATRIX_WIDTH][y].prev) + (world[x][(y + 1) % MATRIX_HEIGHT].prev) +
               (world[(x + MATRIX_WIDTH - 1) % MATRIX_WIDTH][y].prev) +
               (world[x][(y + MATRIX_HEIGHT - 1) % MATRIX_HEIGHT].prev) +
               (world[(x + 1) % MATRIX_WIDTH][(y + 1) % MATRIX_HEIGHT].prev) +
               (world[(x + MATRIX_WIDTH - 1) % MATRIX_WIDTH][(y + 1) % MATRIX_HEIGHT].prev) +
               (world[(x + MATRIX_WIDTH - 1) % MATRIX_WIDTH][(y + MATRIX_HEIGHT - 1) % MATRIX_HEIGHT].prev) +
               (world[(x + 1) % MATRIX_WIDTH][(y + MATRIX_HEIGHT - 1) % MATRIX_HEIGHT].prev);
    }

  public:
    static constexpr auto ID = "Game of Life";

    LifePattern()
        : Pattern(ID)
    {
    }

    void start() override
    {
        generation = 0;
        palette = randomPalette();
    }

    void render() override
    {
        if (generation == 0)
        {
            randomFillWorld();
        }

        // Display current generation
        for (int i = 0; i < MATRIX_WIDTH; i++)
        {
            for (int j = 0; j < MATRIX_HEIGHT; j++)
            {
                GfxBkg(i, j) = ColorFromPalette(palette, world[i][j].hue * 4, world[i][j].brightness);
            }
        }

        // Birth and death cycle
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            for (int y = 0; y < MATRIX_HEIGHT; y++)
            {
                // Default is for cell to stay the same
                if (world[x][y].brightness > 0 && world[x][y].prev == 0)
                    world[x][y].brightness *= 0.9;
                int count = neighbours(x, y);
                if (count == 3 && world[x][y].prev == 0)
                {
                    // A new cell is born
                    world[x][y].alive = 1;
                    world[x][y].hue += 2;
                    world[x][y].brightness = 255;
                }
                else if ((count < 2 || count > 3) && world[x][y].prev == 1)
                {
                    // Cell dies
                    world[x][y].alive = 0;
                }
            }
        }

        // Copy next generation into place
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            for (int y = 0; y < MATRIX_HEIGHT; y++)
            {
                world[x][y].prev = world[x][y].alive;
            }
        }

        generation++;
        if (generation >= 2048)
            generation = 0;
    }

    void backgroundPostProcess() override
    {
        GfxBkg.dim(25);
    }
};
