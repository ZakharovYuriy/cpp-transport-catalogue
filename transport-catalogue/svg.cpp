#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(const Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        if (!points_.empty()) {
            if (points_.end() - points_.begin() > 1) {
                for (auto point_iter = points_.begin(); point_iter < points_.end() - 1; ++point_iter) {
                    out << point_iter->x << "," << point_iter->y << " ";
                }
            }
            out << points_.back().x << "," << points_.back().y << "\" "sv;
        }
        else {
            out << "\" ";
        }
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ------------- Text -------------------
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }
    //text x="35" y="20" dx="0" dy="6" font-size="12" font-family="Verdana" font-weight="bold">
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text "sv;
        out << "x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
        out << " "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " "sv;
        out << "font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " "sv;
            out << "font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_family_.empty()) {
            if (!font_weight_.empty()) {
                out << " "sv;
                out << "font-weight=\""sv << font_weight_ << "\""sv;
            }
        }
        RenderAttrs(context.out);
        out << ">"sv;

        for (const auto& word : data_) {
            if (word == '"') {
                out << "&quot;";
            }
            else if (word == '\'') {
                out << "&apos;";
            }
            else if (word == '<') {
                out << "&lt;";
            }
            else if (word == '>') {
                out << "&gt;";
            }
            else if (word == '&') {
                out << "&amp;";
            }
            else {
                out << word;
            }
        }
        out << "</text>";
    }

    // ------------- Text -------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        vector_obj_ptr_.emplace_back(move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);

        for (auto const& obj : vector_obj_ptr_) {
            obj->Render(ctx);
        }

        out << "</svg>"sv;
    }

}  // namespace svg


std::ostream& operator<< (std::ostream& out, const svg::StrokeLineCap stroke_line_cap)
{
    switch (stroke_line_cap) {
    case svg::StrokeLineCap::BUTT:
        out << "butt";
        break;
    case svg::StrokeLineCap::ROUND:
        out << "round";
        break;
    case svg::StrokeLineCap::SQUARE:
        out << "square";
        break;
    }
    return out;
}

std::ostream& operator<< (std::ostream& out, const svg::StrokeLineJoin stroke_line_join)
{
    switch (stroke_line_join) {
    case svg::StrokeLineJoin::ARCS:
        out << "arcs";
        break;
    case svg::StrokeLineJoin::BEVEL:
        out << "bevel";
        break;
    case svg::StrokeLineJoin::MITER:
        out << "miter";
        break;
    case svg::StrokeLineJoin::MITER_CLIP:
        out << "miter-clip";
        break;
    case svg::StrokeLineJoin::ROUND:
        out << "round";
        break;
    }
    return out;
}