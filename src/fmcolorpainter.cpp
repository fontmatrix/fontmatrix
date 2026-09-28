/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmcolorpainter.h"
#include "fontmatrix_debug.h"

#include <hb-ft.h>
#include <hb-ot.h>

#include <QConicalGradient>
#include <QLinearGradient>
#include <QList>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#if HB_VERSION_ATLEAST(7, 0, 0)

namespace
{
constexpr double fixedOne = 65536.0;

QColor toColor(hb_color_t color)
{
    return QColor(hb_color_get_red(color), hb_color_get_green(color), hb_color_get_blue(color), hb_color_get_alpha(color));
}

/// the outline of a glyph, in font units, as HarfBuzz draws it
void drawMoveTo(hb_draw_funcs_t *, void *data, hb_draw_state_t *, float x, float y, void *)
{
    static_cast<QPainterPath *>(data)->moveTo(x, y);
}
void drawLineTo(hb_draw_funcs_t *, void *data, hb_draw_state_t *, float x, float y, void *)
{
    static_cast<QPainterPath *>(data)->lineTo(x, y);
}
void drawQuadTo(hb_draw_funcs_t *, void *data, hb_draw_state_t *, float cx, float cy, float x, float y, void *)
{
    static_cast<QPainterPath *>(data)->quadTo(cx, cy, x, y);
}
void drawCubicTo(hb_draw_funcs_t *, void *data, hb_draw_state_t *, float c1x, float c1y, float c2x, float c2y, float x, float y, void *)
{
    static_cast<QPainterPath *>(data)->cubicTo(c1x, c1y, c2x, c2y, x, y);
}
void drawClose(hb_draw_funcs_t *, void *data, hb_draw_state_t *, void *)
{
    static_cast<QPainterPath *>(data)->closeSubpath();
}

hb_draw_funcs_t *drawFuncs()
{
    static hb_draw_funcs_t *funcs = [] {
        hb_draw_funcs_t *f = hb_draw_funcs_create();
        hb_draw_funcs_set_move_to_func(f, drawMoveTo, nullptr, nullptr);
        hb_draw_funcs_set_line_to_func(f, drawLineTo, nullptr, nullptr);
        hb_draw_funcs_set_quadratic_to_func(f, drawQuadTo, nullptr, nullptr);
        hb_draw_funcs_set_cubic_to_func(f, drawCubicTo, nullptr, nullptr);
        hb_draw_funcs_set_close_path_func(f, drawClose, nullptr, nullptr);
        hb_draw_funcs_make_immutable(f);
        return f;
    }();
    return funcs;
}

/// a group: what is painted until the group is popped, blended onto the layer below then
struct Layer {
    QImage image;
    std::unique_ptr<QPainter> painter;
};

/// what the callbacks paint on
struct Paint {
    QColor foreground;
    std::vector<std::unique_ptr<Layer>> layers; ///< the bottom one is the glyph, the others open groups

    [[nodiscard]] QPainter *painter() const
    {
        return layers.back()->painter.get();
    }

    /// the area of the image in the coordinates the painter is at now
    [[nodiscard]] QRectF everything() const
    {
        QPainter *p = painter();
        return p->transform().inverted().mapRect(QRectF(layers.back()->image.rect()));
    }

    void fill(const QBrush &brush) const
    {
        QPainter *p = painter();
        p->save();
        p->setPen(Qt::NoPen);
        p->setBrush(brush);
        p->drawRect(everything());
        p->restore();
    }

    [[nodiscard]] QColor colorOf(bool isForeground, hb_color_t color) const
    {
        if (!isForeground)
            return toColor(color);
        QColor c(foreground);
        c.setAlpha(qRound(c.alpha() * hb_color_get_alpha(color) / 255.0));
        return c;
    }
};

QGradient::Spread spreadOf(hb_paint_extend_t extend)
{
    switch (extend) {
    case HB_PAINT_EXTEND_REPEAT:
        return QGradient::RepeatSpread;
    case HB_PAINT_EXTEND_REFLECT:
        return QGradient::ReflectSpread;
    case HB_PAINT_EXTEND_PAD:
        break;
    }
    return QGradient::PadSpread;
}

/**
 * The stops of a colour line, sorted and brought to 0..1: the gradient is
 * then drawn between the places of the first and the last stop, which min
 * and max give as fractions of the way from the start to the end.
 */
bool readColorLine(hb_color_line_t *line, const Paint &paint, QGradientStops &stops, double &min, double &max, QGradient::Spread &spread)
{
    unsigned int count = hb_color_line_get_color_stops(line, 0, nullptr, nullptr);
    if (count == 0)
        return false;
    std::vector<hb_color_stop_t> raw(count);
    hb_color_line_get_color_stops(line, 0, &count, raw.data());
    raw.resize(count);
    std::stable_sort(raw.begin(), raw.end(), [](const hb_color_stop_t &a, const hb_color_stop_t &b) {
        return a.offset < b.offset;
    });
    min = raw.front().offset;
    max = raw.back().offset;
    const double span = max - min;
    stops.clear();
    for (const hb_color_stop_t &stop : raw) {
        const double pos = span > 1e-6 ? (stop.offset - min) / span : 0.0;
        stops << QGradientStop(qBound(0.0, pos, 1.0), paint.colorOf(stop.is_foreground, stop.color));
    }
    if (span <= 1e-6) {
        // one place: the last colour everywhere, as the extend modes all give
        stops = QGradientStops{QGradientStop(0.0, stops.last().second), QGradientStop(1.0, stops.last().second)};
        max = min + 1.0;
    }
    spread = spreadOf(hb_color_line_get_extend(line));
    return true;
}

void pushTransform(hb_paint_funcs_t *, void *data, float xx, float yx, float xy, float yy, float dx, float dy, void *)
{
    QPainter *p = static_cast<Paint *>(data)->painter();
    p->save();
    // applied before what is there: the transform is in the coordinates of the paint it wraps
    p->setTransform(QTransform(xx, yx, xy, yy, dx, dy), true);
}

void popTransform(hb_paint_funcs_t *, void *data, void *)
{
    static_cast<Paint *>(data)->painter()->restore();
}

void pushClipGlyph(hb_paint_funcs_t *, void *data, hb_codepoint_t glyph, hb_font_t *font, void *)
{
    QPainterPath path;
    hb_font_draw_glyph(font, glyph, drawFuncs(), &path);
    path.setFillRule(Qt::WindingFill);
    QPainter *p = static_cast<Paint *>(data)->painter();
    p->save();
    p->setClipPath(path, Qt::IntersectClip);
}

void pushClipRectangle(hb_paint_funcs_t *, void *data, float xmin, float ymin, float xmax, float ymax, void *)
{
    QPainter *p = static_cast<Paint *>(data)->painter();
    p->save();
    p->setClipRect(QRectF(QPointF(xmin, ymin), QPointF(xmax, ymax)), Qt::IntersectClip);
}

void popClip(hb_paint_funcs_t *, void *data, void *)
{
    static_cast<Paint *>(data)->painter()->restore();
}

void paintColor(hb_paint_funcs_t *, void *data, hb_bool_t isForeground, hb_color_t color, void *)
{
    auto *paint = static_cast<Paint *>(data);
    paint->fill(paint->colorOf(isForeground != 0, color));
}

hb_bool_t paintImage(hb_paint_funcs_t *, void *data, hb_blob_t *blob, unsigned int, unsigned int, hb_tag_t format, float, hb_glyph_extents_t *extents, void *)
{
    if (format != HB_PAINT_IMAGE_FORMAT_PNG || !extents)
        return false;
    unsigned int length = 0;
    const char *bytes = hb_blob_get_data(blob, &length);
    const QImage img(QImage::fromData(reinterpret_cast<const uchar *>(bytes), int(length)));
    if (img.isNull())
        return false;
    // height runs downwards in the extents; the painter's y runs up, so the image is turned over into its box
    const QRectF box(extents->x_bearing, extents->y_bearing + extents->height, extents->width, -extents->height);
    QPainter *p = static_cast<Paint *>(data)->painter();
    p->save();
    p->translate(box.center());
    p->scale(1.0, -1.0);
    p->translate(-box.center());
    p->setRenderHint(QPainter::SmoothPixmapTransform, true);
    p->drawImage(box, img);
    p->restore();
    return true;
}

void paintLinearGradient(hb_paint_funcs_t *, void *data, hb_color_line_t *line, float x0, float y0, float x1, float y1, float x2, float y2, void *)
{
    auto *paint = static_cast<Paint *>(data);
    QGradientStops stops;
    double min = 0.0;
    double max = 1.0;
    QGradient::Spread spread = QGradient::PadSpread;
    if (!readColorLine(line, *paint, stops, min, max, spread))
        return;
    // the third point turns the stops: the end is the first point's projection onto the line
    // through the second one that the third point gives the direction of
    const double q2x = x2 - x0;
    const double q2y = y2 - y0;
    const double q1x = x1 - x0;
    const double q1y = y1 - y0;
    const double s = q2x * q2x + q2y * q2y;
    double ex = x1;
    double ey = y1;
    if (s >= 1e-6) {
        const double k = (q2x * q1x + q2y * q1y) / s;
        ex = x1 - k * q2x;
        ey = y1 - k * q2y;
    }
    QLinearGradient gradient(QPointF(x0 + (ex - x0) * min, y0 + (ey - y0) * min), QPointF(x0 + (ex - x0) * max, y0 + (ey - y0) * max));
    gradient.setStops(stops);
    gradient.setSpread(spread);
    paint->fill(gradient);
}

void paintRadialGradient(hb_paint_funcs_t *, void *data, hb_color_line_t *line, float x0, float y0, float r0, float x1, float y1, float r1, void *)
{
    auto *paint = static_cast<Paint *>(data);
    QGradientStops stops;
    double min = 0.0;
    double max = 1.0;
    QGradient::Spread spread = QGradient::PadSpread;
    if (!readColorLine(line, *paint, stops, min, max, spread))
        return;
    // the first stop sits on the first circle, the last on the second: Qt's focal and centre circles
    const QPointF start(x0 + (x1 - x0) * min, y0 + (y1 - y0) * min);
    const QPointF end(x0 + (x1 - x0) * max, y0 + (y1 - y0) * max);
    const double startRadius = r0 + (r1 - r0) * min;
    const double endRadius = r0 + (r1 - r0) * max;
    QRadialGradient gradient(end, endRadius, start, startRadius);
    gradient.setStops(stops);
    gradient.setSpread(spread);
    paint->fill(gradient);
}

void paintSweepGradient(hb_paint_funcs_t *, void *data, hb_color_line_t *line, float cx, float cy, float startAngle, float endAngle, void *)
{
    auto *paint = static_cast<Paint *>(data);
    QGradientStops stops;
    double min = 0.0;
    double max = 1.0;
    QGradient::Spread spread = QGradient::PadSpread;
    if (!readColorLine(line, *paint, stops, min, max, spread))
        return;
    double a0 = startAngle + (endAngle - startAngle) * min;
    double a1 = startAngle + (endAngle - startAngle) * max;
    if (a1 < a0) {
        // Qt's cone runs one way only: the stops are read the other way round
        std::swap(a0, a1);
        QGradientStops turned;
        for (auto it = stops.crbegin(); it != stops.crend(); ++it)
            turned << QGradientStop(1.0 - it->first, it->second);
        stops = turned;
    }
    const double span = std::min(a1 - a0, 2.0 * M_PI);
    QGradientStops around;
    for (const QGradientStop &stop : std::as_const(stops))
        around << QGradientStop(stop.first * span / (2.0 * M_PI), stop.second);
    // the painter's y runs up, Qt's cone counts its angle with y down: the angle is turned over
    QConicalGradient gradient(QPointF(cx, cy), -qRadiansToDegrees(a0));
    gradient.setStops(around);
    gradient.setSpread(spread);
    paint->fill(gradient);
}

void pushGroup(hb_paint_funcs_t *, void *data, void *)
{
    auto *paint = static_cast<Paint *>(data);
    QPainter *below = paint->painter();
    auto layer = std::make_unique<Layer>();
    layer->image = QImage(paint->layers.front()->image.size(), QImage::Format_ARGB32_Premultiplied);
    layer->image.fill(Qt::transparent);
    layer->painter = std::make_unique<QPainter>(&layer->image);
    layer->painter->setRenderHint(QPainter::Antialiasing, true);
    layer->painter->setTransform(below->transform());
    if (below->hasClipping())
        layer->painter->setClipPath(below->clipPath());
    paint->layers.push_back(std::move(layer));
}

QPainter::CompositionMode compositionOf(hb_paint_composite_mode_t mode)
{
    switch (mode) {
    case HB_PAINT_COMPOSITE_MODE_CLEAR:
        return QPainter::CompositionMode_Clear;
    case HB_PAINT_COMPOSITE_MODE_SRC:
        return QPainter::CompositionMode_Source;
    case HB_PAINT_COMPOSITE_MODE_DEST:
        return QPainter::CompositionMode_Destination;
    case HB_PAINT_COMPOSITE_MODE_SRC_OVER:
        return QPainter::CompositionMode_SourceOver;
    case HB_PAINT_COMPOSITE_MODE_DEST_OVER:
        return QPainter::CompositionMode_DestinationOver;
    case HB_PAINT_COMPOSITE_MODE_SRC_IN:
        return QPainter::CompositionMode_SourceIn;
    case HB_PAINT_COMPOSITE_MODE_DEST_IN:
        return QPainter::CompositionMode_DestinationIn;
    case HB_PAINT_COMPOSITE_MODE_SRC_OUT:
        return QPainter::CompositionMode_SourceOut;
    case HB_PAINT_COMPOSITE_MODE_DEST_OUT:
        return QPainter::CompositionMode_DestinationOut;
    case HB_PAINT_COMPOSITE_MODE_SRC_ATOP:
        return QPainter::CompositionMode_SourceAtop;
    case HB_PAINT_COMPOSITE_MODE_DEST_ATOP:
        return QPainter::CompositionMode_DestinationAtop;
    case HB_PAINT_COMPOSITE_MODE_XOR:
        return QPainter::CompositionMode_Xor;
    case HB_PAINT_COMPOSITE_MODE_PLUS:
        return QPainter::CompositionMode_Plus;
    case HB_PAINT_COMPOSITE_MODE_SCREEN:
        return QPainter::CompositionMode_Screen;
    case HB_PAINT_COMPOSITE_MODE_OVERLAY:
        return QPainter::CompositionMode_Overlay;
    case HB_PAINT_COMPOSITE_MODE_DARKEN:
        return QPainter::CompositionMode_Darken;
    case HB_PAINT_COMPOSITE_MODE_LIGHTEN:
        return QPainter::CompositionMode_Lighten;
    case HB_PAINT_COMPOSITE_MODE_COLOR_DODGE:
        return QPainter::CompositionMode_ColorDodge;
    case HB_PAINT_COMPOSITE_MODE_COLOR_BURN:
        return QPainter::CompositionMode_ColorBurn;
    case HB_PAINT_COMPOSITE_MODE_HARD_LIGHT:
        return QPainter::CompositionMode_HardLight;
    case HB_PAINT_COMPOSITE_MODE_SOFT_LIGHT:
        return QPainter::CompositionMode_SoftLight;
    case HB_PAINT_COMPOSITE_MODE_DIFFERENCE:
        return QPainter::CompositionMode_Difference;
    case HB_PAINT_COMPOSITE_MODE_EXCLUSION:
        return QPainter::CompositionMode_Exclusion;
    case HB_PAINT_COMPOSITE_MODE_MULTIPLY:
        return QPainter::CompositionMode_Multiply;
    case HB_PAINT_COMPOSITE_MODE_HSL_HUE:
    case HB_PAINT_COMPOSITE_MODE_HSL_SATURATION:
    case HB_PAINT_COMPOSITE_MODE_HSL_COLOR:
    case HB_PAINT_COMPOSITE_MODE_HSL_LUMINOSITY:
        // QPainter has no HSL modes: the group goes on top as it is
        break;
    }
    return QPainter::CompositionMode_SourceOver;
}

void popGroup(hb_paint_funcs_t *, void *data, hb_paint_composite_mode_t mode, void *)
{
    auto *paint = static_cast<Paint *>(data);
    if (paint->layers.size() < 2)
        return;
    std::unique_ptr<Layer> group(std::move(paint->layers.back()));
    paint->layers.pop_back();
    group->painter->end();
    QPainter *below = paint->painter();
    below->save();
    below->resetTransform();
    below->setCompositionMode(compositionOf(mode));
    below->drawImage(0, 0, group->image);
    below->restore();
}

hb_bool_t customPaletteColor(hb_paint_funcs_t *, void *, unsigned int, hb_color_t *, void *)
{
    return false;
}

hb_paint_funcs_t *paintFuncs()
{
    static hb_paint_funcs_t *funcs = [] {
        hb_paint_funcs_t *f = hb_paint_funcs_create();
        hb_paint_funcs_set_push_transform_func(f, pushTransform, nullptr, nullptr);
        hb_paint_funcs_set_pop_transform_func(f, popTransform, nullptr, nullptr);
        hb_paint_funcs_set_push_clip_glyph_func(f, pushClipGlyph, nullptr, nullptr);
        hb_paint_funcs_set_push_clip_rectangle_func(f, pushClipRectangle, nullptr, nullptr);
        hb_paint_funcs_set_pop_clip_func(f, popClip, nullptr, nullptr);
        hb_paint_funcs_set_color_func(f, paintColor, nullptr, nullptr);
        hb_paint_funcs_set_image_func(f, paintImage, nullptr, nullptr);
        hb_paint_funcs_set_linear_gradient_func(f, paintLinearGradient, nullptr, nullptr);
        hb_paint_funcs_set_radial_gradient_func(f, paintRadialGradient, nullptr, nullptr);
        hb_paint_funcs_set_sweep_gradient_func(f, paintSweepGradient, nullptr, nullptr);
        hb_paint_funcs_set_push_group_func(f, pushGroup, nullptr, nullptr);
        hb_paint_funcs_set_pop_group_func(f, popGroup, nullptr, nullptr);
        hb_paint_funcs_set_custom_palette_color_func(f, customPaletteColor, nullptr, nullptr);
        hb_paint_funcs_make_immutable(f);
        return f;
    }();
    return funcs;
}
} // namespace

hb_font_t *FMColorPainter::paintFont(FT_Face face)
{
    hb_face_t *hbFace = hb_ft_face_create_referenced(face);
    hb_font_t *font = nullptr;
    if (hb_ot_color_has_paint(hbFace)) {
        font = hb_font_create(hbFace);
        hb_ot_font_set_funcs(font);
    }
    hb_face_destroy(hbFace);
    return font;
}

bool FMColorPainter::hasPaint(hb_font_t *font, unsigned int glyph)
{
    return font && hb_ot_color_glyph_has_paint(hb_font_get_face(font), glyph);
}

bool FMColorPainter::paint(hb_font_t *font, unsigned int glyph, double ppem, const QColor &foreground, QImage &image, double &left, double &top)
{
    if (!hasPaint(font, glyph) || ppem <= 0.0)
        return false;
    const double upem = hb_face_get_upem(hb_font_get_face(font));
    if (upem <= 0.0)
        return false;
    const double scale = ppem / upem;

    // the box of the glyph, or the em box around the origin when the font says nothing
    hb_glyph_extents_t extents;
    double xmin = -upem / 2.0;
    double xmax = upem * 1.5;
    double ymin = -upem / 2.0;
    double ymax = upem * 1.5;
    if (hb_font_get_glyph_extents(font, glyph, &extents) && extents.width != 0 && extents.height != 0) {
        xmin = extents.x_bearing;
        xmax = extents.x_bearing + extents.width;
        ymax = extents.y_bearing;
        ymin = extents.y_bearing + extents.height;
    }
    left = std::floor(xmin * scale) - 1.0;
    top = std::ceil(ymax * scale) + 1.0;
    const int width = int(std::ceil(xmax * scale) + 1.0 - left);
    const int height = int(top - (std::floor(ymin * scale) - 1.0));
    if (width <= 0 || height <= 0 || width > 4096 || height > 4096)
        return false;

    Paint paint;
    paint.foreground = foreground;
    auto base = std::make_unique<Layer>();
    base->image = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
    base->image.fill(Qt::transparent);
    base->painter = std::make_unique<QPainter>(&base->image);
    base->painter->setRenderHint(QPainter::Antialiasing, true);
    // font units, y up, the pen at (-left, top) of the image
    base->painter->translate(-left, top);
    base->painter->scale(scale, -scale);
    paint.layers.push_back(std::move(base));

    const hb_color_t fg = HB_COLOR(foreground.blue(), foreground.green(), foreground.red(), foreground.alpha());
    hb_font_paint_glyph(font, glyph, paintFuncs(), &paint, 0, fg);

    // groups left open by a broken paint graph
    while (paint.layers.size() > 1)
        popGroup(nullptr, &paint, HB_PAINT_COMPOSITE_MODE_SRC_OVER, nullptr);
    paint.layers.front()->painter->end();
    image = paint.layers.front()->image;
    return true;
}

#else // HarfBuzz before 7.0: no paint interface

hb_font_t *FMColorPainter::paintFont(FT_Face)
{
    return nullptr;
}

bool FMColorPainter::hasPaint(hb_font_t *, unsigned int)
{
    return false;
}

bool FMColorPainter::paint(hb_font_t *, unsigned int, double, const QColor &, QImage &, double &, double &)
{
    return false;
}

#endif
