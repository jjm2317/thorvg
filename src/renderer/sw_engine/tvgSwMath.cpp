/*
 * Copyright (c) 2020 - 2025 the ThorVG project. All rights reserved.

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

#include "tvgMath.h"
#include "tvgSwCommon.h"


/************************************************************************/
/* Internal Class Implementation                                        */
/************************************************************************/

static float TO_RADIAN(int64_t angle)
{
    return (float(angle) / 65536.0f) * (MATH_PI / 180.0f);
}


/************************************************************************/
/* External Class Implementation                                        */
/************************************************************************/

int64_t mathMean(int64_t angle1, int64_t angle2)
{
    return angle1 + mathDiff(angle1, angle2) / 2;
}


int mathCubicAngle(const SwPoint* base, int64_t& angleIn, int64_t& angleMid, int64_t& angleOut)
{
    auto d1 = base[2] - base[3];
    auto d2 = base[1] - base[2];
    auto d3 = base[0] - base[1];

    if (d1.small()) {
        if (d2.small()) {
            if (d3.small()) {
                angleIn = angleMid = angleOut = 0;
                return -1;  //ignoreable
            } else {
                angleIn = angleMid = angleOut = mathAtan(d3);
            }
        } else {
            if (d3.small()) {
                angleIn = angleMid = angleOut = mathAtan(d2);
            } else {
                angleIn = angleMid = mathAtan(d2);
                angleOut = mathAtan(d3);
            }
        }
    } else {
        if (d2.small()) {
            if (d3.small()) {
                angleIn = angleMid = angleOut = mathAtan(d1);
            } else {
                angleIn = mathAtan(d1);
                angleOut = mathAtan(d3);
                angleMid = mathMean(angleIn, angleOut);
            }
        } else {
            if (d3.small()) {
                angleIn = mathAtan(d1);
                angleMid = angleOut = mathAtan(d2);
            } else {
                angleIn = mathAtan(d1);
                angleMid = mathAtan(d2);
                angleOut = mathAtan(d3);
            }
        }
    }

    auto theta1 = abs(mathDiff(angleIn, angleMid));
    auto theta2 = abs(mathDiff(angleMid, angleOut));

    if ((theta1 < (SW_ANGLE_PI / 8)) && (theta2 < (SW_ANGLE_PI / 8))) return 0; //small size
    return 1;
}


int64_t mathMultiply(int64_t a, int64_t b)
{
    int32_t s = 1;

    //move sign
    if (a < 0) {
        a = -a;
        s = -s;
    }
    if (b < 0) {
        b = -b;
        s = -s;
    }
    int64_t c = (a * b + 0x8000L) >> 16;
    return (s > 0) ? c : -c;
}


int64_t mathDivide(int64_t a, int64_t b)
{
    int32_t s = 1;

    //move sign
    if (a < 0) {
        a = -a;
        s = -s;
    }
    if (b < 0) {
        b = -b;
        s = -s;
    }
    int64_t q = b > 0 ? ((a << 16) + (b >> 1)) / b : 0x7FFFFFFFL;
    return (s < 0 ? -q : q);
}


int64_t mathMulDiv(int64_t a, int64_t b, int64_t c)
{
    int32_t s = 1;

    //move sign
    if (a < 0) {
        a = -a;
        s = -s;
    }
    if (b < 0) {
        b = -b;
        s = -s;
    }
    if (c < 0) {
        c = -c;
        s = -s;
    }
    int64_t d = c > 0 ? (a * b + (c >> 1)) / c : 0x7FFFFFFFL;

    return (s > 0 ? d : -d);
}


void mathRotate(SwPoint& pt, int64_t angle)
{
    if (angle == 0 || pt.zero()) return;

    Point v = pt.toPoint();

    auto radian = TO_RADIAN(angle);
    auto cosv = cosf(radian);
    auto sinv = sinf(radian);

    pt.x = int32_t(nearbyint((v.x * cosv - v.y * sinv) * 64.0f));
    pt.y = int32_t(nearbyint((v.x * sinv + v.y * cosv) * 64.0f));
}


int64_t mathTan(int64_t angle)
{
    if (angle == 0) return 0;
    return int64_t(tanf(TO_RADIAN(angle)) * 65536.0f);
}


int64_t mathAtan(const SwPoint& pt)
{
    if (pt.zero()) return 0;
    return int64_t(tvg::atan2(TO_FLOAT(pt.y), TO_FLOAT(pt.x)) * (180.0f / MATH_PI) * 65536.0f);
}


int64_t mathSin(int64_t angle)
{
    if (angle == 0) return 0;
    return mathCos(SW_ANGLE_PI2 - angle);
}


int64_t mathCos(int64_t angle)
{
    return int64_t(cosf(TO_RADIAN(angle)) * 65536.0f);
}


int64_t mathLength(const SwPoint& pt)
{
    if (pt.zero()) return 0;

    //trivial case
    if (pt.x == 0) return abs(pt.y);
    if (pt.y == 0) return abs(pt.x);

    auto v = pt.toPoint();
    //return static_cast<int64_t>(sqrtf(v.x * v.x + v.y * v.y) * 65536.0f);

    /* approximate sqrt(x*x + y*y) using alpha max plus beta min algorithm.
       With alpha = 1, beta = 3/8, giving results with the largest error less
       than 7% compared to the exact value. */
    if (v.x < 0) v.x = -v.x;
    if (v.y < 0) v.y = -v.y;
    return static_cast<int64_t>((v.x > v.y) ? (v.x + v.y * 0.375f) : (v.y + v.x * 0.375f));
}


void mathSplitCubic(SwPoint* base)
{
    int32_t a, b, c, d;

    base[6].x = base[3].x;
    c = base[1].x;
    d = base[2].x;
    base[1].x = a = (base[0].x + c) >> 1;
    base[5].x = b = (base[3].x + d) >> 1;
    c = (c + d) >> 1;
    base[2].x = a = (a + c) >> 1;
    base[4].x = b = (b + c) >> 1;
    base[3].x = (a + b) >> 1;

    base[6].y = base[3].y;
    c = base[1].y;
    d = base[2].y;
    base[1].y = a = (base[0].y + c) >> 1;
    base[5].y = b = (base[3].y + d) >> 1;
    c = (c + d) >> 1;
    base[2].y = a = (a + c) >> 1;
    base[4].y = b = (b + c) >> 1;
    base[3].y = (a + b) >> 1;
}


void mathSplitLine(SwPoint* base)
{
    base[2] = base[1];
    base[1].x = (base[0].x + base[1].x) >> 1;
    base[1].y = (base[0].y + base[1].y) >> 1;
}


int64_t mathDiff(int64_t angle1, int64_t angle2)
{
    auto delta = angle2 - angle1;

    delta %= SW_ANGLE_2PI;
    if (delta < 0) delta += SW_ANGLE_2PI;
    if (delta > SW_ANGLE_PI) delta -= SW_ANGLE_2PI;

    return delta;
}


SwPoint mathTransform(const Point* to, const Matrix& transform)
{
    auto tx = to->x * transform.e11 + to->y * transform.e12 + transform.e13;
    auto ty = to->x * transform.e21 + to->y * transform.e22 + transform.e23;

    return {TO_SWCOORD(tx), TO_SWCOORD(ty)};
}


bool mathUpdateOutlineBBox(const SwOutline* outline, const RenderRegion& clipBox, RenderRegion& renderBox, bool fastTrack)
{
    if (!outline) return false;

    if (outline->pts.empty() || outline->cntrs.empty()) {
        renderBox.reset();
        return false;
    }

    auto pt = outline->pts.begin();

    auto xMin = pt->x;
    auto xMax = pt->x;
    auto yMin = pt->y;
    auto yMax = pt->y;

    for (++pt; pt < outline->pts.end(); ++pt) {
        if (xMin > pt->x) xMin = pt->x;
        if (xMax < pt->x) xMax = pt->x;
        if (yMin > pt->y) yMin = pt->y;
        if (yMax < pt->y) yMax = pt->y;
    }

    if (fastTrack) {
        renderBox.min = {int32_t(round(xMin / 64.0f)), int32_t(round(yMin / 64.0f))};
        renderBox.max = {int32_t(round(xMax / 64.0f)), int32_t(round(yMax / 64.0f))};
    } else {
        renderBox.min = {xMin >> 6, yMin >> 6};
        renderBox.max = {(xMax + 63) >> 6, (yMax + 63) >> 6};
    }

    renderBox.intersect(clipBox);
    return renderBox.valid();
}
