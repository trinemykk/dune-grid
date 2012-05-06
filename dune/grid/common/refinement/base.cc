// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_COMMON_REFINEMENT_BASE_CC
#define DUNE_GRID_COMMON_REFINEMENT_BASE_CC

// This file is part of DUNE, a Distributed and Unified Numerics Environment
// This file is copyright (C) 2005 Jorrit Fahlke <jorrit@jorrit.de>
// This file is licensed under version 2 of the GNU General Public License,
// with a special "runtime exception."  See COPYING at the top of the source
// tree for the full licence.

/*! @file

   @brief This file contains the parts independent of a particular @ref
         Refinement implementation.

   @verbatim
   $Id$
   @endverbatim
 */

#include <dune/geometry/type.hh>
#include <dune/geometry/genericgeometry/topologytypes.hh>

namespace Dune {

  /*! @addtogroup Refinement Refinement
     @{
   */

  //! @brief This namespace contains the implementation of @ref
  //!        Refinement.
  namespace RefinementImp {

    // /////////////////////////////////
    //
    // Declaration of RefinementImp::Traits
    //

#ifdef DOXYGEN
    // This is just for Doxygen
    /*!
       @brief Mapping from geometryType, CoordType and coerceTo to a
              particular @ref Refinement implementation.

       @tparam topologyId The topology id of the element to refine
       @tparam CoordType  The C++ type of the coordinates
       @tparam coerceToId The topologyId of the subelements
       @tparam dimension  The dimension of the refinement.
       @tparam Dummy      Dummy parameter which can be used for SFINAE, should
                          always be void.

       Each @ref Refinement implementation has to define one or more
       specialisations of this struct to declare what it implements.
       Template class Refinement uses this struct to know which
       implementation it should inherit from.  Since non-type template
       arguments of specializations may not involve template parameters, it is
       often impossible to specify the specialization for all cases directly.
       As the workaround, the template parameter \c Dummy can be used for
       SFINAE with \ref enable_if.

       Each specialisation should contain a single member typedef Imp,
       e.g.:
       @code
       template<class CoordType>
       struct Traits<sphereTopologyId, CoordType, GenericGeometry::CubeToplogy<2>::id, 2>
       {
       typedef SquaringTheCircle::Refinement Imp;
       };
       @endcode
     */
    template<unsigned topologyId, class CoordType,
        unsigned coerceToId, int dimension, class Dummy = void>
    struct Traits
    {
      //! The implementation this specialisation maps to
      typedef SquaringTheCircle::Refinement Imp;
    };


#else // !DOXYGEN

    // Doxygen won't see this

    template< unsigned topologyId
        , class CoordType
        , unsigned coerceToId
        , int dimension
        , class = void
        >
    struct Traits;

#endif // !DOXYGEN
  } // namespace RefinementImp


  /////////////////
  ///
  ///  Static Refinement
  ///

  /*! @brief Wrap each @ref Refinement implementation to get a
             consistent interface

     @tparam topologyId The topology id of the element to refine
     @tparam CoordType  The C++ type of the coordinates
     @tparam coerceToId The topology id of the subelements
     @tparam dimension  The dimension of the refinement.

   */
  template<
      unsigned topologyId
      , class CoordType
      , unsigned coerceToId
      , int dimension_
      >
  class StaticRefinement
    : public RefinementImp::Traits< topologyId
          , CoordType
          , coerceToId
          , dimension_ >::Imp
  {
  public:
#ifdef DOXYGEN
    /*! @brief The Codim struct inherited from the @ref Refinement implementation

       @tparam codimension There is a different struct Codim for each codimension
     */
    template<int codimension>
    struct Codim
    {
      /*! @brief The SubEntityIterator for each codim

         This is @em some sort of type, not necessarily a typedef

       */
      typedef SubEntityIterator;
    };

    //! The VertexIterator of the Refinement
    typedef Codim<dimension>::SubEntityIterator VertexIterator;
    //! The ElementIterator of the Refinement
    typedef Codim<0>::SubEntityIterator ElementIterator;

    /*! @brief The CoordVector of the Refinement

       This is always a typedef to a FieldVector
     */
    typedef CoordVector;
    /*! @brief The IndexVector of the Refinement

       This is always a typedef to a FieldVector
     */
    typedef IndexVector;

    //! Get the number of Vertices
    static int nVertices(int level);
    //! Get a VertexIterator
    static VertexIterator vBegin(int level);
    //! Get a VertexIterator
    static VertexIterator vEnd(int level);

    //! Get the number of Elements
    static int nElements(int level);
    //! Get an ElementIterator
    static ElementIterator eBegin(int level);
    //! Get an ElementIterator
    static ElementIterator eEnd(int level);
#endif //DOXYGEN
    typedef typename RefinementImp::Traits< topologyId, CoordType, coerceToId, dimension_>::Imp RefinementImp;

    using RefinementImp::dimension;

    using RefinementImp::Codim;

    using typename RefinementImp::VertexIterator;
    using typename RefinementImp::CoordVector;

    using typename RefinementImp::ElementIterator;
    using typename RefinementImp::IndexVector;
  };

  namespace RefinementImp {
#ifndef DOXYGEN
    template<GeometryType::BasicType basicType, unsigned dim>
    struct BasicTypeToTopologyId;

    template<unsigned dim>
    struct BasicTypeToTopologyId<GeometryType::simplex, dim> {
      static const unsigned value =
        GenericGeometry::SimplexTopology<dim>::type::id;
    };

    template<unsigned dim>
    struct BasicTypeToTopologyId<GeometryType::cube, dim> {
      static const unsigned value =
        GenericGeometry::CubeTopology<dim>::type::id;
    };

    template<>
    struct BasicTypeToTopologyId<GeometryType::pyramid, 3> {
      static const unsigned value = GenericGeometry::Pyramid<
          GenericGeometry::Prism<
              GenericGeometry::Pyramid<GenericGeometry::Point>
              >
          >::id;
    };

    template<>
    struct BasicTypeToTopologyId<GeometryType::prism, 3> {
      static const unsigned value = GenericGeometry::Prism<
          GenericGeometry::Pyramid<
              GenericGeometry::Pyramid<GenericGeometry::Point>
              >
          >::id;
    };
#endif // !DOXYGEN
  } // namespace RefinementImp

  /*! @} */

} // namespace Dune

#endif //DUNE_GRID_COMMON_REFINEMENT_BASE_CC
