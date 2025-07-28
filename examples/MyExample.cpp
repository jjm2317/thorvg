/*
 * Copyright (c) 2024 - 2025 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Example.h"
#include <cmath>

/************************************************************************/
/* ThorVG Drawing Contents                                              */
/************************************************************************/

struct UserExample : tvgexam::Example
{
    tvg::Shape *circle = nullptr;
    tvg::Shape *triangle = nullptr;
    tvg::Shape *square = nullptr;

    bool content(tvg::Canvas *canvas, uint32_t w, uint32_t h) override
    {
        const uint8_t PINK_R = 255;
        const uint8_t PINK_G = 20;
        const uint8_t PINK_B = 147;

        const float centerX = w / 2.0f;
        const float centerY = h / 2.0f;
        const float shapeSize = 70.0f;
        const float spacing = 130.0f;
        const float strokeWidth = 20.0f;

        circle = tvg::Shape::gen();
        circle->appendCircle(0, 0, shapeSize, shapeSize);

        auto circleGradient = tvg::LinearGradient::gen();
        circleGradient->linear(-shapeSize, -shapeSize, shapeSize, shapeSize);

        tvg::Fill::ColorStop circleStops[3];
        circleStops[0] = {0.0f, 255, 20, 147, 255};
        circleStops[1] = {0.5f, 255, 100, 200, 255};
        circleStops[2] = {1.0f, 255, 20, 147, 255};
        circleGradient->colorStops(circleStops, 3);

        circle->fill(std::move(circleGradient));
        circle->strokeFill(PINK_R, PINK_G, PINK_B);
        circle->strokeWidth(strokeWidth);
        circle->translate(centerX - spacing, centerY);
        canvas->push(circle);

        triangle = tvg::Shape::gen();
        float triangleSize = shapeSize * 1.3f;

        float x1 = 0;
        float y1 = -triangleSize * 0.666f;
        float x2 = -triangleSize * 0.5f;
        float y2 = triangleSize * 0.433f;
        float x3 = triangleSize * 0.5f;
        float y3 = triangleSize * 0.433f;

        triangle->moveTo(x1, y1);
        triangle->lineTo(x2, y2);
        triangle->lineTo(x3, y3);
        triangle->close();

        triangle->strokeFill(PINK_R, PINK_G, PINK_B);
        triangle->strokeWidth(strokeWidth);
        triangle->strokeJoin(tvg::StrokeJoin::Miter);
        triangle->translate(centerX, centerY + 15.0f);
        canvas->push(triangle);

        square = tvg::Shape::gen();
        float squareSize = shapeSize * 1.4f;
        square->appendRect(-squareSize / 2, -squareSize / 2, squareSize, squareSize, 0, 0);

        auto squareGradient = tvg::LinearGradient::gen();
        squareGradient->linear(-squareSize / 2, -squareSize / 2, squareSize / 2, squareSize / 2);

        tvg::Fill::ColorStop squareStops[4];
        squareStops[0] = {0.0f, 255, 20, 147, 255};
        squareStops[1] = {0.3f, 255, 100, 180, 255};
        squareStops[2] = {0.7f, 255, 150, 200, 255};
        squareStops[3] = {1.0f, 255, 20, 147, 255};
        squareGradient->colorStops(squareStops, 4);

        square->fill(std::move(squareGradient));
        square->strokeFill(PINK_R, PINK_G, PINK_B);
        square->strokeWidth(strokeWidth);
        square->strokeJoin(tvg::StrokeJoin::Miter);
        square->translate(centerX + spacing, centerY);
        canvas->push(square);

        return true;
    }

    bool update(tvg::Canvas *canvas, uint32_t elapsed) override
    {
        auto progress = tvgexam::progress(elapsed % 5000, 3.0f, false);

        if (elapsed % 5000 > 3000)
        {
            progress = 1.0f;
        }

        if (circle)
        {
            circle->rotate(360.0f * progress);
            circle->scale(0.5f + 0.5f * std::min(1.0f, progress * 2.5f));
        }

        if (triangle)
        {
            triangle->rotate(-360.0f * progress);
            triangle->scale(0.5f + 0.5f * std::min(1.0f, (progress - 0.2f) * 2.5f));
        }

        if (square)
        {
            square->rotate(180.0f * progress);
            square->scale(0.5f + 0.5f * std::min(1.0f, (progress - 0.4f) * 2.5f));
        }

        canvas->update();
        return true;
    }
};

/************************************************************************/
/* Entry Point                                                          */
/************************************************************************/

int main(int argc, char **argv)
{
    return tvgexam::main(new UserExample, argc, argv);
}
