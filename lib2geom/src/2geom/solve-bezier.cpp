#include <2geom/solver.h>
#include <2geom/choose.h>
#include <2geom/bezier.h>
#include <2geom/point.h>

#include <cmath>
#include <algorithm>

/*** Find the zeros of a Bezier.  The code subdivides until it is happy with the linearity of the
 * function.  This requires an O(degree^2) subdivision for each step, even when there is only one
 * solution.
 * 
 * We try fairly hard to correctly handle multiple roots.
 */

//#define debug(x) do{x;}while(0)
#define debug(x)

namespace Geom {
namespace {

struct Bernsteins
{
    static constexpr size_t MAX_DEPTH = 22;
    std::vector<double> &solutions;

    Bernsteins(std::vector<double> &sol)
        : solutions{sol}
    {}

    double secant(Bezier const &bz);

    void find_bernstein_roots(Bezier const &bz, unsigned depth,
                              double left_t, double right_t);
};

} // namespace

void
Bezier::find_bezier_roots(std::vector<double> &solutions,
                          double left_t, double right_t) const {
    Bezier bz = *this;

    // a constant bezier, even if identically zero, has no roots
    if (bz.isConstant()) {
        return;
    }

    while(bz[0] == 0) {
        debug(std::cout << "deflate\n");
        bz = bz.deflate();
        solutions.push_back(0);
    }
    if (bz.degree() == 1) {
        debug(std::cout << "linear\n");

        if (Geom::sgn(bz[0]) != Geom::sgn(bz[1])) {
            double d = bz[0] - bz[1];
            if(d != 0) {
                double r = bz[0] / d;
                if(0 <= r && r <= 1)
                    solutions.push_back(r);
            }
        }
        return;
    }

    //std::cout << "initial = " << bz << std::endl;
    Bernsteins B(solutions);
    B.find_bernstein_roots(bz, 0, left_t, right_t);
    //std::cout << solutions << std::endl;
}

void Bernsteins::find_bernstein_roots(Bezier const &bz,
                                      unsigned depth,
                                      double left_t,
                                      double right_t)
{
    debug(std::cout << left_t << ", " << right_t << std::endl);
    size_t n_crossings = 0;

    int old_sign = Geom::sgn(bz[0]);
    //std::cout << "w[0] = " << bz[0] << std::endl;
    for (size_t i = 1; i < bz.size(); i++)
    {
        //std::cout << "w[" << i << "] = " << w[i] << std::endl;
        int sign = Geom::sgn(bz[i]);
        if (sign != 0)
        {
            if (sign != old_sign && old_sign != 0)
            {
                ++n_crossings;
            }
            old_sign = sign;
        }
    }
    // if last control point is zero, that counts as crossing too
    if (bz[bz.size() - 1] == 0) {
        ++n_crossings;
    }

    //std::cout << "n_crossings = " << n_crossings << std::endl;
    if (n_crossings == 0)  return; // no solutions here

    if (n_crossings == 1) /* Unique solution  */
    {
        //std::cout << "depth = " << depth << std::endl;
        /* Stop recursion when the tree is deep enough  */
        /* if deep enough, return 1 solution at midpoint  */
        if (depth > MAX_DEPTH)
        {
            //printf("bottom out %d\n", depth);
            const double Ax = right_t - left_t;
            const double Ay = bz.at1() - bz.at0();

            solutions.push_back(left_t - Ax*bz.at0() / Ay);
            return;
        }

        double r = secant(bz);
        solutions.push_back(r*right_t + (1-r)*left_t);
        return;
    }
    /* Otherwise, solve recursively after subdividing control polygon  */
    Bezier::Order o(bz);
    Bezier Left(o), Right = bz;
    double split_t = (left_t + right_t) * 0.5;

    // If subdivision is working poorly, split around the leftmost root of the derivative
    if (depth > 2) {
        debug(std::cout << "derivative mode\n");
        Bezier dbz = derivative(bz);
    
        debug(std::cout << "initial = " << dbz << std::endl);
        std::vector<double> dsolutions = dbz.roots(Interval(left_t, right_t));
        debug(std::cout << "dsolutions = " << dsolutions << std::endl);
        
        double dsplit_t = 0.5;
        if(!dsolutions.empty()) {
            dsplit_t = dsolutions[0];
            if (dsplit_t < EPSILON || dsplit_t > 1 - EPSILON) {
                // Avoid infinite recursion if making no progress.
                dsplit_t = 0.5;
            }
            split_t = left_t + (right_t - left_t)*dsplit_t;
            debug(std::cout << "split_value = " << bz(split_t) << std::endl);
            debug(std::cout << "splitting around " << dsplit_t << " = " 
                  << split_t << "\n");
        
        }
        std::pair<Bezier, Bezier> LR = bz.subdivide(dsplit_t);
        Left = LR.first;
        Right = LR.second;
    } else {
        // split at midpoint, because it is cheap
        Left[0] = Right[0];
        for (size_t i = 1; i < bz.size(); ++i)
        {
            for (size_t j = 0; j < bz.size()-i; ++j)
            {
                Right[j] = (Right[j] + Right[j+1]) * 0.5;
            }
            Left[i] = Right[0];
        }
    }
    debug(std::cout << "Solution is exactly on the subdivision point.\n");
    debug(std::cout << Left << " , " << Right << std::endl);
    Left = reverse(Left);
    while(Right.order() > 0 && fabs(Right[0]) <= 1e-10) {
        debug(std::cout << "deflate\n");
        Right = Right.deflate();
        Left = Left.deflate();
        solutions.push_back(split_t);
    }
    Left = reverse(Left);
    if (Right.order() > 0) {
        debug(std::cout << Left << " , " << Right << std::endl);
        find_bernstein_roots(Left, depth+1, left_t, split_t);
        find_bernstein_roots(Right, depth+1, split_t, right_t);
    }
}

double Bernsteins::secant(Bezier const &bz) {
    double s = 0, t = 1;
    double e = 1e-14;
    int side = 0;
    double r, fs = bz.at0(), ft = bz.at1();

    for (size_t n = 0; n < 100; ++n)
    {
        r = (fs*t - ft*s) / (fs - ft);
        if (fabs(t-s) < e * fabs(t+s)) {
            debug(std::cout << "error small " << fabs(t-s) 
                      << ", accepting solution " << r 
                  << "after " << n << "iterations\n");
            return r;
        }

        double fr = bz.valueAt(r);

        if (fr * ft > 0)
        {
            t = r; ft = fr;
            if (side == -1) fs /= 2;
            side = -1;
        }
        else if (fs * fr > 0)
        {
            s = r;  fs = fr;
            if (side == +1) ft /= 2;
            side = +1;
        }
        else break;
    }
    return r;
}

} // namespace Geom

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
