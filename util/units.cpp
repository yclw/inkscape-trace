// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape Units
 *
 * Copyright (C) 2013-2025 AUTHORS
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cmath>
#include <cerrno>
#include <iomanip>
#include <iostream>
#include <utility>
#include <unordered_map>
#include <glib.h>
#include <glibmm/regex.h>
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>

#include <2geom/coord.h>

#include "util/units.h"
#include "util-string/ustring-format.h"

namespace {
    std::string get_units_xml_path() {
        return "units.xml";
    }
}

using Inkscape::Util::UNIT_TYPE_DIMENSIONLESS;
using Inkscape::Util::UNIT_TYPE_LINEAR;
using Inkscape::Util::UNIT_TYPE_RADIAL;
using Inkscape::Util::UNIT_TYPE_FONT_HEIGHT;

namespace
{

#define MAKE_UNIT_CODE(a, b) \
    ((((unsigned)(a) & 0xdf) << 8) | ((unsigned)(b) & 0xdf))

enum UnitCode {
    UNIT_CODE_PX = MAKE_UNIT_CODE('p','x'),
    UNIT_CODE_PT = MAKE_UNIT_CODE('p','t'),
    UNIT_CODE_PC = MAKE_UNIT_CODE('p','c'),
    UNIT_CODE_MM = MAKE_UNIT_CODE('m','m'),
    UNIT_CODE_CM = MAKE_UNIT_CODE('c','m'),
    UNIT_CODE_IN = MAKE_UNIT_CODE('i','n'),
    UNIT_CODE_EM = MAKE_UNIT_CODE('e','m'),
    UNIT_CODE_EX = MAKE_UNIT_CODE('e','x'),
    UNIT_CODE_PERCENT = MAKE_UNIT_CODE('%',0)
};

// TODO: convert to constexpr in C++11, so that the above constants can be eliminated
inline unsigned make_unit_code(char const *str) {
    if (!str || str[0] == 0) return 0;
    return MAKE_UNIT_CODE(str[0], str[1]);
}


// This must match SVGLength::Unit
unsigned const svg_length_lookup[] = {
    0,
    UNIT_CODE_PX,
    UNIT_CODE_PT,
    UNIT_CODE_PC,
    UNIT_CODE_MM,
    UNIT_CODE_CM,
    UNIT_CODE_IN,
    UNIT_CODE_EM,
    UNIT_CODE_EX,
    UNIT_CODE_PERCENT
};



// maps unit codes obtained from their abbreviations to their SVGLength unit indexes
typedef std::unordered_map<unsigned, SVGLength::Unit> UnitCodeLookup;

UnitCodeLookup make_unit_code_lookup()
{
    UnitCodeLookup umap;
    for (unsigned i = 1; i < G_N_ELEMENTS(svg_length_lookup); ++i) {
        umap[svg_length_lookup[i]] = static_cast<SVGLength::Unit>(i);
    }
    return umap;
}

UnitCodeLookup const unit_code_lookup = make_unit_code_lookup();



typedef std::unordered_map<Glib::ustring, Inkscape::Util::UnitType> TypeMap;

/** A std::map that gives the data type value for the string version.
 * @todo consider hiding map behind hasFoo() and getFoo() type functions. */
TypeMap make_type_map()
{
    TypeMap tmap;
    tmap["DIMENSIONLESS"] = UNIT_TYPE_DIMENSIONLESS;
    tmap["LINEAR"] = UNIT_TYPE_LINEAR;
    tmap["RADIAL"] = UNIT_TYPE_RADIAL;
    tmap["FONT_HEIGHT"] = UNIT_TYPE_FONT_HEIGHT;
    // Note that code was not yet handling LINEAR_SCALED, TIME, QTY and NONE

    return tmap;
}

TypeMap const type_map = make_type_map();

} // namespace

namespace Inkscape {
namespace Util {

class UnitParser : public Glib::Markup::Parser
{
public:
    typedef Glib::Markup::Parser::AttributeMap AttrMap;
    typedef Glib::Markup::ParseContext Ctx;

    UnitParser(UnitTable *table);
    ~UnitParser() override = default;

protected:
    void on_start_element(Ctx &ctx, Glib::ustring const &name, AttrMap const &attrs) override;
    void on_end_element(Ctx &ctx, Glib::ustring const &name) override;
    void on_text(Ctx &ctx, Glib::ustring const &text) override;

    bool parse_bool(Glib::ustring const &ret) {
        auto b = ret.lowercase();
        return b == "y" || b == "true";
    }
public:
    UnitTable *tbl;
    bool primary;
    bool skip;
    Unit unit;
    std::optional<UnitMetric> metric;
    bool is_div;
};

UnitParser::UnitParser(UnitTable *table) :
    tbl(table),
    primary(false),
    skip(false)
{
}

#define BUFSIZE (255)

Unit::Unit() :
    type(UNIT_TYPE_DIMENSIONLESS), // should this or NONE be the default?
    factor(1.0),
    name(),
    name_plural(),
    abbr(),
    description()
{
}

Unit::Unit(UnitType type,
           double factor,
           Glib::ustring name,
           Glib::ustring name_plural,
           Glib::ustring abbr,
           Glib::ustring description)
    : type(type)
    , factor(factor)
    , name(std::move(name))
    , name_plural(std::move(name_plural))
    , abbr(std::move(abbr))
    , description(std::move(description))
{
    g_return_if_fail(factor <= 0);
}

void Unit::clear()
{
    *this = Unit();
}

int Unit::defaultDigits() const
{
    int factor_digits = int(log10(factor));
    if (factor_digits < 0) {
        g_warning("factor = %f, factor_digits = %d", factor, factor_digits);
        g_warning("factor_digits < 0 - returning 0");
        factor_digits = 0;
    }
    return factor_digits;
}

bool Unit::compatibleWith(Unit const *u) const
{
    // Percentages
    if (type == UNIT_TYPE_DIMENSIONLESS || u->type == UNIT_TYPE_DIMENSIONLESS) {
        return true;
    }

    // Other units with same type
    if (type == u->type) {
        return true;
    }

    // Different, incompatible types
    return false;
}
bool Unit::compatibleWith(Glib::ustring const &u) const
{
    return compatibleWith(UnitTable::get().getUnit(u));
}

bool Unit::operator==(Unit const &other) const
{
    return (type == other.type && name.compare(other.name) == 0);
}

int Unit::svgUnit() const
{
    char const *astr = abbr.c_str();
    unsigned code = make_unit_code(astr);

    UnitCodeLookup::const_iterator u = unit_code_lookup.find(code);
    if (u != unit_code_lookup.end()) {
        return u->second;
    }
    return 0;
}

double Unit::convert(double from_dist, Unit const *to) const
{
    // Percentage
    if (to->type == UNIT_TYPE_DIMENSIONLESS) {
        return from_dist * to->factor;
    }   
      
    // Incompatible units
    if (type != to->type) {
        return -1; 
    }   
      
    // Compatible units
    return from_dist * factor / to->factor;
}
double Unit::convert(double from_dist, Glib::ustring const &to) const
{
    return convert(from_dist, UnitTable::get().getUnit(to));
} 
double Unit::convert(double from_dist, char const *to) const
{
    return convert(from_dist, UnitTable::get().getUnit(to));
}

UnitMetric const *Unit::getUnitMetric() const
{
    return UnitTable::get().getUnitMetric(metric_name);
}


Unit UnitTable::_empty_unit;

UnitTable::UnitTable()
{
    load(get_units_xml_path());
}
UnitTable::UnitTable(std::string const &filename)
{
    load(filename);
}

UnitTable::~UnitTable()
{
    for (auto & iter : _unit_map)
    {
        delete iter.second;
    }
}

void UnitTable::addMetric(UnitMetric const &m, bool primary)
{
    _metric_map[m.name] = m;
    if (primary) {
        _default_metric = m.name;
    }
}

UnitMetric const *UnitTable::getUnitMetric(Glib::ustring const &name) const
{
    if (auto f = _metric_map.find(name); f != _metric_map.end()) {
        return &(f->second);
    }
    if (name == _default_metric) {
        g_warning("No default ruler metric found!");
        return nullptr;
    }
    return getUnitMetric(_default_metric);
}

void UnitTable::addUnit(Unit const &u, bool primary)
{
    _unit_map[make_unit_code(u.abbr.c_str())] = new Unit(u);
    if (primary) {
        _primary_unit[u.type] = u.abbr;
    }
}

Unit const *UnitTable::getUnit(char const *abbr) const
{
    UnitCodeMap::const_iterator f = _unit_map.find(make_unit_code(abbr));
    if (f != _unit_map.end()) {
        return &(*f->second);
    }
    return &_empty_unit;
}

Unit const *UnitTable::getUnit(Glib::ustring const &unit_abbr) const
{
    return getUnit(unit_abbr.c_str());
}
Unit const *UnitTable::getUnit(SVGLength::Unit u) const
{
    if (u == 0 || u > SVGLength::LAST_UNIT) {
        return &_empty_unit;
    }

    UnitCodeMap::const_iterator f = _unit_map.find(svg_length_lookup[u]);
    if (f != _unit_map.end()) {
        return &(*f->second);
    }
    return &_empty_unit;
}

Unit const *UnitTable::findUnit(double factor, UnitType type) const
{
    const double eps = factor * 0.01; // allow for 1% deviation

    UnitCodeMap::const_iterator cit = _unit_map.begin();
    while (cit != _unit_map.end()) {
        if (cit->second->type == type) {
            if (Geom::are_near(cit->second->factor, factor, eps)) {
                // unit found!
                break;
            }
        }
        ++cit;
    }

    if (cit != _unit_map.end()) {
        return cit->second;
    } else {
        return getUnit(_primary_unit[type]);
    }
}

Quantity UnitTable::parseQuantity(Glib::ustring const &q) const
{
    Glib::MatchInfo match_info;

    // Extract value
    double value = 0;
    Glib::RefPtr<Glib::Regex> value_regex = Glib::Regex::create("[-+]*[\\d+]*[\\.,]*[\\d+]*[eE]*[-+]*\\d+");
    if (value_regex->match(q, match_info)) {
        std::istringstream tmp_v(match_info.fetch(0).raw());
        tmp_v >> value;
    }
    int start_pos, end_pos;
    match_info.fetch_pos(0, end_pos, start_pos);
    end_pos = q.size() - start_pos;
    Glib::ustring u = q.substr(start_pos, end_pos);

    // Extract unit abbreviation
    Glib::ustring abbr;
    Glib::RefPtr<Glib::Regex> unit_regex = Glib::Regex::create("[A-z%]+");
    if (unit_regex->match(u, match_info)) {
        abbr = match_info.fetch(0);
    }

    Quantity qty(value, abbr);
    return qty;
}

/* UNSAFE while passing around pointers to the Unit objects in this table 
bool UnitTable::deleteUnit(Unit const &u)
{
    bool deleted = false;
    // Cannot delete the primary unit type since it's
    // used for conversions
    if (u.abbr != _primary_unit[u.type]) {
        UnitCodeMap::iterator iter = _unit_map.find(make_unit_code(u.abbr.c_str()));
        if (iter != _unit_map.end()) {
            delete (*iter).second;
            _unit_map.erase(iter);
            deleted = true;
        }
    }
    return deleted;
}
*/

bool UnitTable::hasUnit(Glib::ustring const &unit) const
{
    UnitCodeMap::const_iterator iter = _unit_map.find(make_unit_code(unit.c_str()));
    return (iter != _unit_map.end());
}

UnitTable::UnitMap UnitTable::units(UnitType type) const
{
    UnitMap submap;
    for (auto iter : _unit_map) {
        if (iter.second->type == type) {
            submap.insert(UnitMap::value_type(iter.second->abbr, *iter.second));
        }
    }

    return submap;
}

Glib::ustring UnitTable::primary(UnitType type) const
{
    return _primary_unit[type];
}

bool UnitTable::load(std::string const &filename) {
    UnitParser uparser(this);
    Glib::Markup::ParseContext ctx(uparser);

    try {
        Glib::ustring unitfile = Glib::file_get_contents(filename);
        ctx.parse(unitfile);
        ctx.end_parse();
    } catch (Glib::FileError const &e) {
        g_warning("Units file %s is missing: %s\n", filename.c_str(), e.what());
        return false;
    } catch (Glib::MarkupError const &e) {
        g_warning("Problem loading units file '%s': %s\n", filename.c_str(), e.what());
        return false;
    }
    return true;
}

UnitTable &UnitTable::get()
{
    static UnitTable instance;
    return instance;
}

/*
bool UnitTable::save(std::string const &filename) {
    g_warning("UnitTable::save(): not implemented");

    return false;
}
*/

void UnitParser::on_start_element(Ctx &/*ctx*/, Glib::ustring const &name, AttrMap const &attrs)
{
    if (name == "metric") {
        metric = std::make_optional<UnitMetric>();
        primary = false;

        if (auto f = attrs.find("name"); f != attrs.end()) {
            metric->name = f->second;
        } else {
            metric.reset();
            g_warning("Skipping unit metric with no name");
        }
        if (auto f = attrs.find("default"); f != attrs.end()) {
            primary = parse_bool(f->second);
        }
    } else if (name == "tic") {
        is_div = false;
        if (auto f = attrs.find("div"); f != attrs.end()) {
            is_div = parse_bool(f->second);
        }
    }
    if (name == "unit") {
        // reset for next use
        unit.clear();
        primary = false;
        skip = false;

        if (auto f = attrs.find("type"); f != attrs.end()) {
            Glib::ustring type = f->second;
            TypeMap::const_iterator tf = type_map.find(type);
            if (tf != type_map.end()) {
                unit.type = tf->second;
            } else {
                g_warning("Skipping unknown unit type '%s'.\n", type.c_str());
                skip = true;
            }
        }
        if (auto f = attrs.find("pri"); f != attrs.end()) {
            primary = parse_bool(f->second);
        }
        if (auto f = attrs.find("metric"); f != attrs.end()) {
            unit.metric_name = f->second;
        }
    }
}

void UnitParser::on_text(Ctx &ctx, Glib::ustring const &text)
{
    Glib::ustring element = ctx.get_element();
    if (element == "name") {
        unit.name = text;
    } else if (element == "plural") {
        unit.name_plural = text;
    } else if (element == "abbr") {
        unit.abbr = text;
    } else if (element == "factor") {
        // TODO make sure we use the right conversion
        unit.factor = std::stod(text.raw());
    } else if (element == "description") {
        unit.description = text;
    } else if (element == "tic" && metric) {
        auto tic = std::stod(text.raw());
        metric->ruler_scale.push_back(tic);
        if (is_div) {
            metric->subdivide.push_back(tic);
        }
    }
}

void UnitParser::on_end_element(Ctx &/*ctx*/, Glib::ustring const &name)
{
    if (name == "unit" && !skip) {
        tbl->addUnit(unit, primary);
    } else if (name == "metric") {
        if (metric) {
            tbl->addMetric(*std::move(metric), primary);
            metric.reset();
        }
    }
}

Quantity::Quantity(double q, Unit const *u)
  : unit(u)
  , quantity(q)
{
}
Quantity::Quantity(double q, Glib::ustring const &u)
  : unit(UnitTable::get().getUnit(u.c_str()))
  , quantity(q)
{
}
Quantity::Quantity(double q, char const *u)
  : unit(UnitTable::get().getUnit(u))
  , quantity(q)
{
}

bool Quantity::compatibleWith(Unit const *u) const
{
    return unit->compatibleWith(u);
}
bool Quantity::compatibleWith(Glib::ustring const &u) const
{
    return compatibleWith(u.c_str());
}
bool Quantity::compatibleWith(char const *u) const
{
    return compatibleWith(UnitTable::get().getUnit(u));
}

double Quantity::value(Unit const *u) const
{
    return convert(quantity, unit, u);
}
double Quantity::value(Glib::ustring const &u) const
{
    return value(u.c_str());
}
double Quantity::value(char const *u) const
{
    return value(UnitTable::get().getUnit(u));
}

Glib::ustring Quantity::string(Unit const *u) const {
    return Inkscape::ustring::format_classic(std::fixed, std::setprecision(2), value(u)) + " " + u->abbr;
}
Glib::ustring Quantity::string(Glib::ustring const &u) const {
    return string(UnitTable::get().getUnit(u.c_str()));
}
Glib::ustring Quantity::string() const {
    return string(unit);
}

double Quantity::convert(double from_dist, Unit const *from, Unit const *to)
{
    return from->convert(from_dist, to);
}
double Quantity::convert(double from_dist, Glib::ustring const &from, Unit const *to)
{
    return convert(from_dist, UnitTable::get().getUnit(from.c_str()), to);
}
double Quantity::convert(double from_dist, Unit const *from, Glib::ustring const &to)
{
    return convert(from_dist, from, UnitTable::get().getUnit(to.c_str()));
}
double Quantity::convert(double from_dist, Glib::ustring const &from, Glib::ustring const &to)
{
    return convert(from_dist, UnitTable::get().getUnit(from.c_str()), UnitTable::get().getUnit(to.c_str()));
}
double Quantity::convert(double from_dist, char const *from, char const *to)
{
    return convert(from_dist, UnitTable::get().getUnit(from), UnitTable::get().getUnit(to));
}

bool Quantity::operator<(Quantity const &rhs) const
{
    if (unit->type != rhs.unit->type) {
        g_warning("Incompatible units");
        return false;
    }
    return quantity < rhs.value(unit);
}
bool Quantity::operator==(Quantity const &other) const
{
    /** \fixme  This is overly strict. I think we should change this to:
    if (unit->type != other.unit->type) {
        g_warning("Incompatible units");
        return false;
    }
    return are_near(quantity, other.value(unit));
    */
    return (*unit == *other.unit) && (quantity == other.quantity);
}

} // namespace Util
} // namespace Inkscape

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
