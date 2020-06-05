#include <gui.h>

cairo_t *gui::Element::cr = nullptr;

void loading_bar(cairo_t *cr, float x, float y, float r, float theta) {
    cairo_arc(cr, x, y, r, theta, 2 * M_PI);
    cairo_stroke(cr);
}