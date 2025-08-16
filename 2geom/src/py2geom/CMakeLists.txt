SET(2GEOM_BOOST_PYTHON_SRC
etc.cpp
point.cpp
interval.cpp
transforms.cpp
rect.cpp
line.cpp
circle.cpp
ellipse.cpp
conic.cpp
crossing.cpp
sbasis.cpp
bezier.cpp
linear.cpp
pw.cpp
d2.cpp
parser.cpp
path.cpp
ray.cpp
#convexcover.cpp
py2geom.cpp
# curves
#curve.cpp
#bezier-curve.cpp
)

IF (WIN32)
    SET(BUILD_BOOST_PYTHON_STATIC FALSE)
ELSE (WIN32)
    SET(BUILD_BOOST_PYTHON_STATIC FALSE)
ENDIF (WIN32)
IF (BUILD_BOOST_PYTHON_STATIC)
  SET(BOOST_PYTHON_SRC "C:/boost_1_42_0/libs/python/src")
  #define BOOST_PYTHON_STATIC_LIB
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_PYTHON_STATIC_LIB")
  SET(2GEOM_BOOST_PYTHON_SRC
    ${2GEOM_BOOST_PYTHON_SRC}
    ${BOOST_PYTHON_SRC}/dict.cpp
    ${BOOST_PYTHON_SRC}/errors.cpp
    ${BOOST_PYTHON_SRC}/exec.cpp
    ${BOOST_PYTHON_SRC}/import.cpp
    ${BOOST_PYTHON_SRC}/list.cpp
    ${BOOST_PYTHON_SRC}/long.cpp
    ${BOOST_PYTHON_SRC}/module.cpp
    ${BOOST_PYTHON_SRC}/numeric.cpp
    ${BOOST_PYTHON_SRC}/object_operators.cpp
    ${BOOST_PYTHON_SRC}/object_protocol.cpp
    ${BOOST_PYTHON_SRC}/slice.cpp
    ${BOOST_PYTHON_SRC}/str.cpp
    ${BOOST_PYTHON_SRC}/tuple.cpp
    ${BOOST_PYTHON_SRC}/wrapper.cpp

    ${BOOST_PYTHON_SRC}/converter/arg_to_python_base.cpp
    ${BOOST_PYTHON_SRC}/converter/builtin_converters.cpp
    ${BOOST_PYTHON_SRC}/converter/from_python.cpp
    ${BOOST_PYTHON_SRC}/converter/registry.cpp
    ${BOOST_PYTHON_SRC}/converter/type_id.cpp

    ${BOOST_PYTHON_SRC}/object/class.cpp
    ${BOOST_PYTHON_SRC}/object/enum.cpp
    ${BOOST_PYTHON_SRC}/object/function.cpp
    ${BOOST_PYTHON_SRC}/object/function_doc_signature.cpp
    ${BOOST_PYTHON_SRC}/object/inheritance.cpp
    ${BOOST_PYTHON_SRC}/object/iterator.cpp
    ${BOOST_PYTHON_SRC}/object/life_support.cpp
    ${BOOST_PYTHON_SRC}/object/pickle_support.cpp
    ${BOOST_PYTHON_SRC}/object/stl_iterator.cpp
  )
ENDIF (BUILD_BOOST_PYTHON_STATIC)

IF(PYCAIRO_FOUND)
    SET(2GEOM_BOOST_PYTHON_SRC
    ${2GEOM_BOOST_PYTHON_SRC}
    cairo-helpers.cpp
    )
ENDIF(PYCAIRO_FOUND)


OPTION(2GEOM_BOOST_PYTHON
  "Build a python binding with Boost.Python"
  OFF)
IF(2GEOM_BOOST_PYTHON)
	FIND_PACKAGE(Python3 COMPONENTS Development Interpreter REQUIRED)

	SET(Boost_DEBUG TRUE)
        SET(Boost_REALPATH FALSE)
        FIND_PACKAGE(Boost 1.42.0 REQUIRED)
	FIND_PACKAGE(Boost REQUIRED COMPONENTS python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR})

        IF (WIN32)
            SET_TARGET_PROPERTIES(py2geom PROPERTIES SUFFIX ".pyd")
        ELSEIF (APPLE)
	    SET(CMAKE_MACOSX_RPATH FALSE)
	    SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
        ENDIF(WIN32)

	INCLUDE_DIRECTORIES( src/ ${Python3_INCLUDE_DIRS} ${Boost_INCLUDE_DIR} )
        ADD_LIBRARY(py2geom SHARED ${2GEOM_BOOST_PYTHON_SRC})
        SET_TARGET_PROPERTIES(py2geom PROPERTIES PREFIX "_")

        IF (BUILD_BOOST_PYTHON_STATIC)
            TARGET_LINK_LIBRARIES(py2geom 2geom ${Python3_LIBRARIES})
        ELSE (BUILD_BOOST_PYTHON_STATIC)
	    TARGET_LINK_LIBRARIES(py2geom 2geom ${Boost_LIBRARIES} ${Python3_LIBRARIES})
        ENDIF (BUILD_BOOST_PYTHON_STATIC)

        set_property(TARGET 2geom PROPERTY POSITION_INDEPENDENT_CODE ON)  # we need -fPIC to link py2geom against 2geom

        IF(PYCAIRO_FOUND)
            TARGET_LINK_LIBRARIES(py2geom ${Cairo_LIBRARIES})
        ENDIF(PYCAIRO_FOUND)

	INSTALL(TARGETS py2geom DESTINATION "${Python3_SITEARCH}/py2geom")
        INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py" DESTINATION "${Python3_SITEARCH}/py2geom")
ENDIF(2GEOM_BOOST_PYTHON)



