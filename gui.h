#include <cmath>
#include <cairo/cairo.h>
#include <vector>
#include <functional>
#include <string>
#include <pango/pangocairo.h>

namespace gui {

struct Color {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    double a = 0.0;
};

struct Style {
    Color color;
    Color backgroundColor;
    Color borderColor;
    double borderWidth;
    double padding;
};

class Element {
protected:
    std::vector<Element*> children_;
    Style style_;
    PangoLayout *layout_ = nullptr;

public:
    static cairo_t *cr;

    Element *parent_ = nullptr;
    float x = 0;
    float y = 0;
    float w = 0;
    float h = 0;

    double contentWidth = 0.0;
    double contentHeight = 0.0;

    Element() = default;
    
    Element(std::vector<Element*> children): children_{children} {};
    
    Element(float w, float h) {
        this->w = w;
        this->h = h;
    }
    Element(float x, float y, float w, float h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }

    void setPangoLayout(const std::string &s) {
        PangoLayout *layout;    
        PangoFontDescription *desc;
        layout = pango_cairo_create_layout (cr);
        pango_layout_set_text (layout, s.c_str(), s.size());
        desc = pango_font_description_from_string ("Verdana,Sans");
        pango_font_description_set_absolute_size(desc, pango_units_from_double(12));
        pango_layout_set_font_description (layout, desc);
        pango_font_description_free (desc);
        this->layout_ = layout;
    }

    void addChild(Element *child) {
        children_.push_back(child);
        child->setParent(this);
    }

    Style& style() {
        return style_;
    }

    virtual void layoutChildren() {
        
        if (this->parent_ && this->w == 0.0) {
            this->w = this->parent_->w - 2 * this->parent_->style().padding - 2 * this->parent_->style().borderWidth;
        }

        pango_layout_set_width(layout_, pango_units_from_double(this->w));
        pango_layout_set_alignment (layout_, PANGO_ALIGN_CENTER);

        float acc_height = this->style().borderWidth + this->style().padding;
        float max_width = 0;
        for (auto *child: children_) {
            
            child->layoutChildren();

            child->x = this->style().borderWidth + this->style().padding;
            child->y = acc_height;
            acc_height += child->h;
        }

        acc_height += this->style().borderWidth + this->style().padding;
        this->contentHeight = acc_height;
        this->contentWidth = max_width;

        if (layout_ != nullptr) {
            this->contentHeight += pango_layout_get_height(layout_);
        }

        if (this->h == 0.0) {
            this->h = this->contentHeight;
        }
    }

    void setParent(Element *parent) {
        parent_ = parent;
    }

    virtual void draw(cairo_t *cr, float dx, float dy) {

        Color background = style().backgroundColor;
        Color border = style().borderColor;
        Color color = style().color;

        cairo_set_source_rgba(cr, background.r, background.b, background.g, background.a);
        cairo_rectangle(cr,x + dx + style().borderWidth, y + dy + style().borderWidth, w - 2 * style().borderWidth, h - 2 * style().borderWidth);
        cairo_fill(cr);

        cairo_set_line_width(cr, style().borderWidth);
        cairo_set_source_rgba(cr, border.r, border.b, border.g, border.a);
        cairo_rectangle(cr,x + dx + style().borderWidth / 2, y + dy + style().borderWidth / 2, w - style().borderWidth, h - style().borderWidth);
        cairo_stroke(cr);

        for (auto *child: children_) {
            child->draw(cr, this->x + dx, this->y + dy);
        }

        cairo_set_source_rgba(cr, color.r, color.b, color.g, color.a);

        if (layout_ != nullptr) {
            cairo_translate(cr, this->x + dx + style().borderWidth + style().padding, this->y + dy + style().borderWidth + style().padding);
            pango_cairo_show_layout (cr, layout_);
            cairo_translate(cr, -(this->x + dx + style().borderWidth + style().padding), -(this->y + dy + style().borderWidth + style().padding));
        }

    }

    virtual ~Element() = default;
};

class FlexLayout: public Element {
public:

    enum Direction {
        ROW, ROW_REVERSE, COLUMN, COLUMN_REVERSE
    };

    enum JustifyContent {
        FLEX_START, FLEX_END, CENTER, SPACE_BETWEEN, SPACE_AROUND, SPACE_EVENLY
    };  

private:
    Direction direction_ = Direction::ROW;
    JustifyContent justify_ = JustifyContent::FLEX_START;

public:

    FlexLayout() = default;
    FlexLayout(std::vector<Element*> children): Element{children} {}

    void setDirection(Direction direction) {
        this->direction_ = direction;
    }

    void setJustifyContent(JustifyContent justify) {
        this->justify_ = justify;
    }

    virtual void layoutChildren() override {

        if (this->parent_ && this->w == 0.0) {
            this->w = this->parent_->w - 2 * this->parent_->style().padding - 2 * this->parent_->style().borderWidth;
        }

        this->contentWidth = this->w - 2 * (this->style().borderWidth + this->style().padding);

        pango_layout_set_width(layout_, pango_units_from_double(this->contentWidth));

        double sum_child_width = 0.0;
        double sum_child_height = 0.0;

        double max_child_width = 0.0;
        double max_child_height = 0.0;
        
        for (auto *child: children_) {

            child->layoutChildren();

            sum_child_width += child->w;
            sum_child_height += child->h;

            if (child->w > max_child_width) {
                max_child_width = child->w;
            }

            if (child->h > max_child_height) {
                max_child_height = child->h;
            }
        }

        if (layout_ != nullptr) {
            int width;
            int height;
            pango_layout_get_size (layout_, &width, &height);
            if (PANGO_PIXELS(height) > max_child_height) {
                max_child_height = PANGO_PIXELS(height);
            }
        }

        if (this->h == 0.0) {
            this->h = max_child_height + 2 * this->style().padding + 2 * this->style().borderWidth;
        }

        this->contentHeight = max_child_height;


        float acc_width = this->style().borderWidth + this->style().padding;
        float acc_height = this->style().borderWidth + this->style().padding;
        float acc_gap = 0.0;

        float free_width = this->contentWidth - sum_child_width;
        float free_height = this->contentHeight - sum_child_height;

        if (this->justify_ == JustifyContent::CENTER) {
            acc_width = (this->w - sum_child_width) / 2.0;
            acc_height = (this->h - sum_child_height) / 2.0;
        } else if (this->justify_ == JustifyContent::FLEX_END) {
            acc_width = this->w - sum_child_width - acc_width;
            acc_height = this->h - sum_child_height - acc_height;
        } else if (this->justify_ == JustifyContent::SPACE_AROUND) {
            acc_height += free_height / (2 * children_.size());
            acc_width += free_width / (2 * children_.size());
            acc_gap = free_width / (children_.size());
        } else if (this->justify_ == JustifyContent::SPACE_EVENLY) {
            acc_height += free_height / (children_.size() + 1);
            acc_width += free_width / (children_.size() + 1);
            acc_gap =  free_width / (children_.size() + 1);
        } else if (this->justify_ == JustifyContent::SPACE_BETWEEN) {
            acc_gap = free_width / (children_.size() - 1);
        }

        for (auto *child: children_) {
            child->x = acc_width;
            child->y = this->style().borderWidth + this->style().padding;
            acc_width += child->w + acc_gap;
        };

        /*
        if (this->h == 0) {
            this->h = max_child_height;
        }

        if (this->parent_ && this->w == 0) {
            this->w = this->parent_->w;
        }

        float free_width = this->parent_->w - sum_child_width;
        float free_height = this->h - sum_child_height;

        float acc_width = 0.0;
        float acc_height = 0.0;

    
        if (this->justify_ == JustifyContent::CENTER) {
            acc_width = (this->w - sum_child_height) / 2.0;
            acc_height = (this->h - sum_child_width) / 2.0;
        } else if (this->justify_ == JustifyContent::SPACE_AROUND) {
            ch = fh / (2 * children_.size());
            cw = fw / (2 * children_.size());
        } else if (this->justify_ == JustifyContent::SPACE_EVENLY) {
            ch = fh / (children_.size() + 1);
            cw = fw / (children_.size() + 1);
        } else if (this->justify_ == JustifyContent::FLEX_END) {
            cw = this->w - sw;
            ch = this->h - sh;
        }
        

        for (auto *child: children_) {
            child->x = acc_width;
            acc_width += child->w;
            child->y = 0;
            this->h = child->h > this->h ? child->h: this->h;

            
            if (direction_ == Direction::ROW || direction_ == Direction::ROW_REVERSE) {
                
                if (direction_ == Direction::ROW) {
                    child->x = cw;
                } else if (direction_ == Direction::ROW_REVERSE) {
                    child->x = this->w - cw - child->w;
                } 

                if (justify_ == JustifyContent::SPACE_BETWEEN) {
                    cw += fw / (children_.size() - 1) + child->w;
                } else if (justify_ == JustifyContent::SPACE_AROUND) {
                    cw += fw / (children_.size()) + child->w;
                } else if (justify_ == JustifyContent::SPACE_EVENLY) {
                    cw += fw / (children_.size() + 1) + child->w;
                } else {
                    cw += child->w;
                }

                child->y = 0;
                this->h = child->h > this->h ? child->h: this->h;

            } else {

                if (direction_ == Direction::COLUMN) {
                    child->y = ch;  
                } else if (direction_ == Direction::COLUMN_REVERSE) {
                    child->y = this->h - ch - child->h;
                }

                if (justify_ == JustifyContent::SPACE_BETWEEN) {
                    ch += fh / (children_.size() - 1) + child->h;
                } else if (justify_ == JustifyContent::SPACE_AROUND) {
                    ch += fh / (children_.size()) + child->h;
                } else if (justify_ == JustifyContent::SPACE_EVENLY) {
                    ch += fh / (children_.size() + 1) + child->h;
                } else {
                    ch += child->h;
                }

                child->x = 0;
                this->w = child->w > this->w ? child->w: this->w;
            }
        }

        */

    }

    virtual ~FlexLayout() {
        for (auto *child: children_) {
            delete child;
        }
    }
};



};