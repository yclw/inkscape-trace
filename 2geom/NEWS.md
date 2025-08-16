lib2geom v1.1.0
===============

2Geom v1.1 is not ABI compatible with v1.0, it switches from
`boost::optional` to `std::optional`.

Changes:

- Add `Geom::Parallelogram`
- Add `Geom::PathIteratorSink::inPath()`
- Add `Geom::are_near_rel()` for `Geom::Point`
- Move headers to `include` directory
- Make build system git submodule friendly
- Fix Python 3 support (py2geom)
- Remove Python 2 support (py2geom)


lib2geom v1.0.0
===============

2geom is a C++ library of mathematics for paths, curves, and other
geometric calculations, designed to be well suited for vector graphics:
Bézier curves, conics, paths, intersections, transformations, and basic
geometries.

Originally developed to restructure and improve path data structures in
Inkscape, this library's codebase has been maintained and shipped as
part of the professional vector graphics software for over a decade.

The major contributors to the 2geom library are Nathan Hurst, Michael
G. Sloan, Krzysztof Kosiński, Johan B. C. Engelen, MenTaLguY, Aaron
Spike, Marco Cechetti and JF Barraud.

Work on this release has focused on updating the 2geom source control,
build, test and packaging systems for both Linux and Windows. The py2geom
python extension package has been restored and improvements have been
made to overall code stabilization and quality.

The primary motivation for 2geom's 1.0 release is to support a future
Inkscape 1.0 launch.

With this evolution to a distinct package, the 2geom team is seeking
new opportunities to collaborate with individuals and projects
interested in using this proven tool.

