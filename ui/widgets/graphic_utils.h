//
// Created by brant on 2/27/24.
//

#pragma once

#include "text_helpers.h"
#include <any>
#include <functional>
#include "utils.h"
#include "icons/icons_embed.h"
#include <utility>
#include <list>
#include <rmkit.h>

/*
 * ╭----╮
 * |    |
 * ╰----╯
 *
 * Draws all four rounded corners as described by the center and radius of the top-left corner,
 * and the offset to the bottom-right corner
 *
 * Optionally, you can specify a color for the border, its thickness, and a gradient
 * Thickness expands outward from the centerpoint, and the gradient is a sigmoid function (utils::sigmoid)
 *
 * colors are specified as a float in the range (0,1) where 0 is black and 1 is white (see color::from_float)
 *
 * This is an adaptation of the circle outline algorithm at framebuffer::draw_circle_outline
 * The gradient is accomplished by drawing multiple 1px arcs with diminishing color in 1px steps away from center
 *
 * We only actually compute one eighth of the circle and mirror it to all the other octants
 */
inline void drawRoundedCorners(int x0, int y0, int ox, int oy, int radius, framebuffer::FB *fb,
                               float grayfColor = 0, uint stroke = 1, bool gradient = false,
                               float grayfendColor = 1,
                               float expA = -20.f, float coefB = 8, float alphaMask = 0.9f) {
    int x = 0;
    int y = radius;
    int d = -(radius >> 1);
    int w = stroke;
    int h = stroke;
    auto color = color::from_float(grayfColor);

    //since we use _draw_rect_fast, we need to manually mark the area dirty
    fb->update_dirty(fb->dirty_area, x0 - radius - stroke, y0 - radius - stroke);
    fb->update_dirty(fb->dirty_area, x0 + ox + radius + stroke, y0 + oy + radius + stroke);
    fb->waveform_mode = WAVEFORM_MODE_GC16;

    if (!gradient) {
        while (x <= y) {
            //q4
            fb->_draw_rect_fast(-x + x0 - stroke, -y + y0 - stroke, w, h, color);
            fb->_draw_rect_fast(-y + x0 - stroke, -x + y0 - stroke, w, h, color);
            //q1
            fb->_draw_rect_fast(x + x0 + ox, -y + y0 - stroke, w, h, color);
            fb->_draw_rect_fast(y + x0 + ox, -x + y0 - stroke, w, h, color);
            //q2
            fb->_draw_rect_fast(x + x0 + ox, y + y0 + oy, w, h, color);
            fb->_draw_rect_fast(y + x0 + ox, x + y0 + oy, w, h, color);
            //q3
            fb->_draw_rect_fast(-x + x0 - stroke, y + y0 + oy, w, h, color);
            fb->_draw_rect_fast(-y + x0 - stroke, x + y0 + oy, w, h, color);

            if (d <= 0) {
                x++;
                d += x;
            } else {
                y--;
                d -= y;
            }
        }
    } else {
        auto dc = abs(grayfendColor - grayfColor) / (float) stroke;
        for (uint si = 0; si <= stroke; si++) {
            auto fc = utils::sigmoid(grayfColor + dc * si, expA, coefB);
            if (fc < alphaMask)
                drawRoundedCorners(x0, y0, ox, oy, radius + si, fb, fc);
        }
    }
}

//draws a box with rounded corners with some style options
//calls into drawRoundedCorners first, then draws the lines connecting the arcs
//gradient is based on a sigmoid function (see utils::sigmoid)
//colors are specified as a float in the range (0,1) where 0 is black and 1 is white (see color::from_float)
inline void drawRoundedBox(int x0, int y0, int w, int h, int radius, framebuffer::FB *fb,
                           int stroke = 1, float grayfColor = 0, int shrink = 0, bool gradient = false,
                           float grayfendColor = 1,
                           float expA = -20.f, float coefB = 8, float alphaThreshold = 0.9f) {
    int sx = x0 + shrink;
    int sy = y0 + shrink;
    int dx = w - (2 * shrink);
    int dy = h - (2 * shrink);

    if (!gradient) {
        auto color = color::from_float(grayfColor);
        drawRoundedCorners(sx, sy, dx, dy, radius, fb, grayfColor, stroke);
        fb->_draw_rect_fast(sx - stroke - radius, sy, stroke, dy, color);
        fb->_draw_rect_fast(sx + dx + radius, sy, stroke, dy, color);
        fb->_draw_rect_fast(sx, sy - stroke - radius, dx, stroke, color);
        fb->_draw_rect_fast(sx, sy + dy + radius, dx, stroke, color);
    } else {
        drawRoundedCorners(sx, sy, dx, dy, radius, fb, grayfColor, stroke, gradient, grayfendColor, expA, coefB, alphaThreshold);
        float dc = abs(grayfendColor - grayfColor) / (float) stroke;
        for (int i = 0; i <= stroke; i++) {
            auto fc = utils::sigmoid(grayfColor + (dc * i), expA, coefB);
            if (fc >= alphaThreshold)
                continue;   //don't break, the curve may change later in the stroke
            auto color = color::from_float(fc);
            //left
            fb->_draw_rect_fast(sx - i - radius - 1, sy, 1, dy, color);
            //right
            fb->_draw_rect_fast(sx + dx + radius + i, sy, 1, dy, color);
            //top
            fb->_draw_rect_fast(sx, sy - i - radius - 1, dx, 1, color);
            //bottom
            fb->_draw_rect_fast(sx, sy + dy + i + radius, dx, 1, color);
        }
    }
}

