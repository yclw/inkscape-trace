# Code Health:
 - Rewrite old tests using Google Test Framework.
 - Add more unit tests. Ideally, every method should be tested.
 - More Doxygen documentation. Document algorithms used to implement each
    nontrivial method, preferably with pictures.
 - Coding style.
 - Rewrite or remove old junk.

# Primitives:
 - Add uniform points - Geom::UPoint.
 - Add projective transformations - Geom::Projective.
 - Add rational Bezier fragments / curves - Geom::D2U<Bezier>

# Shapes:
 - Geom::Triangle
 - Geom::Quad
 - Geom::Polygon
 - Function that computes an Affine that maps between triangles.
 - Function that maps between Rects.
 - Function that maps between Ellipses (no skew).

# Path / PathVector:
 - Built in support for arc length parametrization, computed on demand.
 - Boolean operations accessible via operators: &, |, / (or -), ^
 - Uncrossing.
 - Conversion from nonzero winding rule to even-odd winding rule.
 - Dashing - requires arc length parametrization.
 - Offset.
 - Stroke-to-path.
 - Minkowski sum.
 - Functions to facilitate node editing without conversion to a node list.
 - Approximation of arbitrary paths with cubic Beziers, quadratic Beziers
    or line segments, available as a path sink.
 - Approximation by lines and circular / elliptical arcs.
 - Support for per-node and per-segment user data?

# Fragments:
 - Convert all uses of std::vector<Point> to D2<Bezier> where applicable.
 - Consider adding push_back() for incremental building of Beziers.
 - Implement Bezier versions of SBasis methods.

# Toys:
 - Better Bezier / SBasis handles.
 - Use GTK 3 to provide widgets such as checkboxes and buttons (?)

# Other:
 - sweeper.h: add variants of Sweeper that accept void Item
    and add a version that does sweepline over two lists rather
    than one.
 - Rewrite conic section code to match coding style.
 - Rewrite QuadTree and RTree to make them actually usable.
 - BSP tree.
 - Interval tree - Geom::IntervalSet, Geom::IntervalMap<T>
 - Geom::MultiInterval
 - Using the above, add regions, as seen in Cairo.
 - Add some basic 3D functionality, enough to draw a 3D box.
 - Write GDB pretty printers for all core classes. See the directory "tools"
 - Clothoid support (clothoids where the curvature is a not-necessarily linear
   function, perhaps piecewise SBasis, add fitting)
 - Special geometric shapes (stuff like circles, oriented rectangles, spirals,
   triangles, regular polygons, inkscape-like polygons, etc)
 - Constraint system for advanced handle stuff
 - complete and tidy up intersection apis
 - complete offset implementation: at this stage only the hard part has been
 implemented
 - conic sections: we currently support line and ellipse from xAx form, add
 parabola and hyperbola; intersection
 - NURBS (2geom handles division approximation of polynomials, code just needs
   to be added to convert between standard forms (say x/w, y/w in bernstein and
     pw<d2<sb>>(t))
 - T-mesh form for 2D (and higher?) piecewise polynomials
 - enhance contour tracing algorithm (sb-2d-solver)
 - transform an svg stroke in an outline (extends offset curve code to handle
   SVG options)
 - variable width curve tracing (variant on path along path idea)
 - collinear tangent (find a line that smoothly connects two curves); extension
 of bezier clipping idea
 - transform a path into a single curve (fitting) - improvements required
 - exploiting modern architecture features (cache coherence, multi-core, packet
   instruction set, gpgpu)
 - finish boolops (use faster and more robust sweep-window algorithm, add
   support for dB @ dB, dB @ B, B @ B where @ is any of the 16 boolean operators
   that make sense)
 - bucket fill (variant of bool ops for bucket filling)
 - path topology and numerical soundness graph
