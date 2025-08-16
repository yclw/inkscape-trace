#include <2geom/path.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

class DrawToy: public Toy {
    PointSetHandle hand;
    //Knot : Och : Och : Knot : Och : Och : Knot : Och : Och : ...
    void draw(cairo_t *cr, std::ostringstream */*notify*/, int /*width*/, int /*height*/, bool save, std::ostringstream*) override {
        if(!save) {
            cairo_set_source_rgba (cr, 0, 0.5, 0, 1);
            cairo_set_line_width (cr, 1);
            for(unsigned i = 0; i < hand.pts.size(); i+=3) {
                draw_circ(cr, hand.pts[i]);
                draw_number(cr, hand.pts[i], i/3);
            }
            cairo_set_source_rgba (cr, 0, 0, 0.5, 1);
            for(unsigned i = 2; i < hand.pts.size(); i+=3) {
                draw_circ(cr, hand.pts[i]);
                draw_circ(cr, hand.pts[i-1]);
            }

            cairo_set_source_rgba (cr, 0.5, 0, 0, 1);
            for(unsigned i = 3; i < hand.pts.size(); i+=3) {
                draw_line_seg(cr, hand.pts[i-2], hand.pts[i-3]);
                draw_line_seg(cr, hand.pts[i], hand.pts[i-1]);
            }
        }
        cairo_set_source_rgba (cr, 0, 0, 0, 1);
        Geom::Path pb;
        if(hand.pts.size() > 3) {
            pb.start(hand.pts[0]);
            for(unsigned i = 1; i < hand.pts.size() - 3; i+=3) {
                pb.appendNew<Geom::CubicBezier>(hand.pts[i], hand.pts[i+1], hand.pts[i+2]);
            }
        }
        cairo_path(cr, pb);
        cairo_stroke(cr);
    }
    void mouse_pressed(Geom::Point const &pos, unsigned button, unsigned modifiers) override
    {
        selected = nullptr;
        int close_i = 0;
        float close_d = 1000;
        for(unsigned i = 0; i < hand.pts.size(); i+=1) {
            if (Geom::distance(pos, hand.pts[i]) < close_d) {
                close_d = Geom::distance(pos, hand.pts[i]);
                close_i = i;
            }
        }
        if (close_d < 5) {
             if (button==3)
                 hand.pts.erase(hand.pts.begin() + close_i);
             else {
                 selected = &hand;
                 hit_data = (void*)(intptr_t)close_i;
             }
        } else {
             if (button == 1) {
                 if (hand.pts.size() > 0) {
                     if (hand.pts.size() == 1) {
                         hand.pts.push_back((hand.pts[0] * 2 + pos) / 3);
                         hand.pts.push_back((hand.pts[0] + pos * 2) / 3);
                     } else {
                         Geom::Point prev = hand.pts[hand.pts.size() - 1];
                         Geom::Point curve = prev - hand.pts[hand.pts.size() - 2];
                         hand.pts.push_back(prev + curve);
                         hand.pts.push_back(pos + curve);
                     }
                 }
                 hand.pts.push_back(pos);
             } else {
                 selected = &hand;
                 hit_data = (void*)(intptr_t)close_i;
             }
        }
    }

    void mouse_moved(Geom::Point const &pos, unsigned modifiers) override
    {
        if (modifiers & (GDK_BUTTON1_MASK) && selected) {
            // NOTE this is completely broken.
            int hd = 0;
            if (hd % 3 == 0) {
                Geom::Point diff = pos - hand.pts[hd];
                if(int(hand.pts.size() - 1) > hd) hand.pts[hd + 1] += diff;
                if(hd != 0) hand.pts[hd - 1] += diff; 
            }
            Toy::mouse_moved(pos, modifiers);
        }
    }

    bool should_draw_numbers() override { return false; }

public:
    DrawToy()
    {
        handles.push_back(&hand);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new DrawToy());
    return 0;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
