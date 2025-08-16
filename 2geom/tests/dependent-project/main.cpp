#include <2geom/2geom.h>
#include <iostream>
#include "my_lib.h"

int main() {
    Geom::Rect rect1(0, 0, 1, 1);
    Geom::Rect rect2(0.5, 0.5, 1.5, 1.5);

    std::cout << sum_of_three_points(Geom::Point(1, 1), Geom::Point(1, 2), Geom::Point(2, 3));

    return rect1.intersects(rect2) ? 0 : 1;
}
