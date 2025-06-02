#pragma once

class AuroraDropPattern final : public Pattern
{
    // x,y, & time values
    uint32_t x = 0, y = 0, v_time = 0, hue_time = 0, hxy = 0;

    // Play with the values of the variables below and see what kinds of effects they
    // have!  More octaves will make things slower.

    // how many octaves to use for the brightness and hue functions
    uint8_t octaves = 1;
    uint8_t hue_octaves = 3;

    // the 'distance' between points on the x and y axis
    int xscale = 57771;
    int yscale = 57771;

    // the 'distance' between x/y points for the hue noise
    int hue_scale = 1;

    // how fast we move through time & hue noise
    int time_speed = 1111;
    int hue_speed = 31;

    // adjust these values to move along the x or y axis between frames
    int x_speed = 331;
    int y_speed = 1111;

    bool started = false;

public:
    static constexpr auto ID = "AuroraDrop";

    AuroraDropPattern() : Pattern(ID)
    {
    }

    // ------------------ start -------------------
    void start(PatternContext& ctx) override
    {
        if (!started)
        {
            started = true;

            random16_set_seed(8934);
            random16_add_entropy(analogRead(3));

            hxy = (static_cast<uint32_t>(random16()) << 16) + static_cast<uint32_t>(random16());
            x = (static_cast<uint32_t>(random16()) << 16) + static_cast<uint32_t>(random16());
            y = (static_cast<uint32_t>(random16()) << 16) + static_cast<uint32_t>(random16());
            v_time = (static_cast<uint32_t>(random16()) << 16) + static_cast<uint32_t>(random16());
            hue_time = (static_cast<uint32_t>(random16()) << 16) + static_cast<uint32_t>(random16());

            ctx.noise.randomize();
        }
    }


    // --------------------- draw frame -------------------------
    void render(PatternContext& ctx) override
    {
        ctx.noise.noise_x += x_speed;
        ctx.noise.noise_y += y_speed;
        ctx.noise.noise_z += hue_speed;
        ctx.noise.fill();

        for (uint8_t i = 0; i < MATRIX_WIDTH; i++)
        {
            for (uint8_t j = 0; j < MATRIX_HEIGHT; j++)
            {
                const uint8_t color = ctx.noise(i, j);
                ctx.leds(i, j) = ColorFromPalette(ctx.targetPalette,
                                                  color,
                                                  ctx.audio.energy8Scaled);
            }
        }

        // Matrix::ApplyCanvasQ(Matrix::canvasQ.data(), 0, 0);
        // Matrix::ApplyCanvasQ(Matrix::canvasQ.data(), MATRIX_CENTER_X / 2, 0); // need to mirror/flip these
        // Matrix::ApplyCanvasQ(Matrix::canvasQ.data(), 0, MATRIX_CENTER_Y / 2);
        // Matrix::ApplyCanvasQ(Matrix::canvasQ.data(), MATRIX_CENTER_X / 2, MATRIX_CENTER_Y / 2);

        blur2d(ctx.leds.data(),
               MATRIX_WIDTH / 2,
               MATRIX_HEIGHT / 2,
               ctx.audio.energy8Scaled);

        //Matrix::ApplyCanvasH(Matrix::canvasH, MATRIX_CENTER_X, 0);
        //Matrix::ApplyCanvasH(Matrix::canvasH, 0, MATRIX_CENTER_Y);
        //Matrix::ApplyCanvasH(Matrix::canvasH, MATRIX_CENTER_X, MATRIX_CENTER_Y);

        //Matrix::Kaleidoscope2();
        //blur2d(Matrix::leds, MATRIX_WIDTH > 255 ? 255 : MATRIX_WIDTH, MATRIX_HEIGHT > 255 ? 255 : MATRIX_HEIGHT, 128);


        // adjust the intra-frame time values
    }
};
