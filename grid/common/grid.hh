// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_HH
#define DUNE_GRID_HH

/** \file
    \brief Different resources needed by all grid implementations
 */
#include <iostream>
#include <string>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/helpertemplates.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/geometrytype.hh>
#include <dune/common/capabilities.hh>

namespace Dune {

  // compares a and b and if they are equal then throw and NotImplemented
  // exception
#if 0
#ifndef NDEBUG
#define CHECK_INTERFACE_IMPLEMENTATION(a,b) \
  if((a) == (b)) \
    DUNE_THROW(NotImplemented,"Interface method not implemented!");
#else
#define CHECK_INTERFACE_IMPLEMENTATION(a,b)
#endif
#endif
#define CHECK_INTERFACE_IMPLEMENTATION(a,b)

  /**
     @defgroup Grid Grid

     The Dune Grid module defines a general interface to a hierarchical
     finite element mesh.  The interface is independent of dimension and
     element type.
   */

  /**
     @defgroup GridInterface Grid Application API
     @ingroup Grid
     \brief Interfaces needed when working with a \ref Grid "Dune::Grid"
   */

  /**
     @defgroup GridDevel Grid Developer API
     @ingroup Grid
     \brief Interfaces needed to implement a new \ref Grid "Dune::Grid"
   */

  /**
     @defgroup GridImplementations Implementations Overview
     @ingroup Grid
     \brief A List of the different Implementations of the Dune Grid Interface.
   */

  /**
     @ingroup GridInterface
   */

  enum AdaptationState {
    NONE ,   //!< notin' to do and notin' was done
    COARSEN, //!< entity could be coarsend in adaptation step
    REFINED  //!< enity was refined in adaptation step
  };


  /*! Parameter to be used for the communication functions
   */
  enum InterfaceType {
    InteriorBorder_InteriorBorder_Interface=0,
    InteriorBorder_All_Interface=1,
    Overlap_OverlapFront_Interface=2,
    Overlap_All_Interface=3,
    All_All_Interface=4
  };

  /*! Parameter to be used for the parallel level iterators
   */
  enum PartitionIteratorType {
    Interior_Partition=0,
    InteriorBorder_Partition=1,
    Overlap_Partition=2,
    OverlapFront_Partition=3,
    All_Partition=4,
    Ghost_Partition=5
  };


  /*! Define a type for communication direction parameter
   */
  enum CommunicationDirection {
    ForwardCommunication,
    BackwardCommunication
  };

  /*! Attributes used in the generic overlap model
   */
  enum PartitionType {
    InteriorEntity=0,     //!< all interior entities
    BorderEntity=1  ,     //!< on boundary between interior and overlap
    OverlapEntity=2 ,     //!< all entites lying in the overlap zone
    FrontEntity=3  ,      //!< on boundary between overlap and ghost
    GhostEntity=4         //!< ghost entities
  };

  //! provide names for the partition types
  inline std::string PartitionName(PartitionType type)
  {
    switch(type) {
    case InteriorEntity :
      return "interior";
    case BorderEntity :
      return "border";
    case OverlapEntity :
      return "overlap";
    case FrontEntity :
      return "front";
    case GhostEntity :
      return "ghost";
    default :
      DUNE_THROW(NotImplemented, "name of unknown partition type requested");
    }
  }

  /*! GridIndexType specifies which Index of the Entities of the grid
        should be used, i.e. globalIndex() or index()
   */
  enum GridIndexType { GlobalIndex , //!< use globalIndex() of entity
                       LevelIndex  ,  //!< use index() of entity
                       LeafIndex    //!< use index() of entity
  };

  //************************************************************************
  // G R I D E R R O R
  //************************************************************************

  /*!
     exceptions in Dune grid modules.
   */

  class GridError : public Exception {};

  // Forward Declarations
  template<int mydim, int cdim, class GridImp,template<int,int,class> class GeometryImp> class Geometry;
  // dim is necessary because Entity will be specialized for codim==0 _and_ codim==dim
  // EntityImp gets GridImp as 3rd template parameter to distinguish between const and mutable grid
  template<int codim, int dim, class GridImp,template<int,int,class> class EntityImp> class Entity;
  template<class GridImp, class EntityPointerImp> class EntityPointer;
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp> class LevelIterator;
  template<class GridImp, template<class> class IntersectionIteratorImp> class IntersectionIterator;
  template<class GridImp, template<class> class HierarchicIteratorImp> class HierarchicIterator;
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LeafIteratorImp> class LeafIterator;
  template<class GridImp> class GenericLeafIterator;
  template<class GridImp, class IndexSetIteratorImp, class IndexSetImp> class IndexSet;
  template<class GridImp, class IdSetImp, class IdTypeImp> class IdSet;


  //************************************************************************
  // G R I D
  //************************************************************************

  /**
     \brief Grid interface class

     This class should actually be called GridInterface since it defines the
     basic interface for all grid classes

     template parameters:
   * <tt>dim</tt> precifies the dimesion of the grid.
   * <tt>dimworld</tt> precifies the dimension of the surrounding space, this can be
       different from dim, if the grid is defined on a manifold .
   * <tt>ct</tt> field type of the world vector space.
   * <tt>GridFamily</tt> trait class providing all information
       associated with the grid implementation.
   */
  template< int dim, int dimworld, class ct, class GridFamily>
  class Grid {
    typedef typename GridFamily::Traits::Grid GridImp;
    typedef Grid<dim,dimworld,ct,GridFamily> ThisType;
  public:

    template <int cd>
    struct Codim
    {
      // IMPORTANT: Codim<codim>::Geometry == Geometry<dim-codim,dimworld>
      typedef typename GridFamily::Traits::template Codim<cd>::Geometry Geometry;
      typedef typename GridFamily::Traits::template Codim<cd>::LocalGeometry LocalGeometry;

      typedef typename GridFamily::Traits::template Codim<cd>::Entity Entity;

      typedef typename GridFamily::Traits::template Codim<cd>::LevelIterator LevelIterator;

      typedef typename GridFamily::Traits::template Codim<cd>::LeafIterator LeafIterator;

      typedef typename GridFamily::Traits::template Codim<cd>::EntityPointer EntityPointer;

      template <PartitionIteratorType pitype>
      struct Partition
      {
        typedef typename GridFamily::Traits::template Codim<cd>::template Partition<pitype>::LevelIterator LevelIterator;
        typedef typename GridFamily::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator LeafIterator;
      };

      typedef typename GridFamily::Traits::HierarchicIterator HierarchicIterator;

      typedef typename GridFamily::Traits::IntersectionIterator IntersectionIterator;

      typedef typename GridFamily::Traits::LevelIndexSet LevelIndexSet;
      typedef typename GridFamily::Traits::LeafIndexSet LeafIndexSet;
      typedef typename GridFamily::Traits::GlobalIdSet GlobalIdSet;
      typedef typename GridFamily::Traits::LocalIdSet LocalIdSet;
    };

    enum {
      //! \brief The dimension of the grid
      dimension=dim
    };

    enum {
      //! \brief The dimension of the world the grid lives in.
      dimensionworld=dimworld
    };

    //! Define type used for coordinates in grid module
    typedef ct ctype;

    /*! Return maximum level defined in this grid. Levels are numbered
       0 ... maxLevel with 0 the coarsest level.
     */
    int maxLevel() const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)() const) &(ThisType::maxLevel)),
        ((int (GridImp::*)() const) &(GridImp::maxLevel)));
      return asImp().maxLevel();
    }

    //! Return number of grid entities of a given codim on a given level
    int size (int level, int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int,int) const) &(ThisType::size)),
        ((int (GridImp::*)(int,int) const) &(GridImp::size )));
      return asImp().size(level,codim);
    }

    //! number of leaf entities per codim in this process
    int size (int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int) const) &(ThisType::size)),
        ((int (GridImp::*)(int) const) &(GridImp::size)));
      return asImp().size(codim);
    }

    //! number of entities per level and geometry type in this process
    int size (int level, GeometryType type) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int,GeometryType) const) &(ThisType::size)),
        ((int (GridImp::*)(int,GeometryType) const) &(GridImp::size)));
      return asImp().size(level,type);
    }

    //! number of leaf entities per geometry type in this process
    int size (GeometryType type) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(GeometryType) const) &(ThisType::size)),
        ((int (GridImp::*)(GeometryType) const) &(GridImp::size)));
      return asImp().size(type);
    }

    //! return size (= distance in graph) of overlap region
    int overlapSize (int level, int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int,int) const) &(ThisType::overlapSize)),
        ((int (GridImp::*)(int,int) const) &(GridImp::overlapSize )));
      return asImp().overlapSize(level,codim);
    }

    //! return size (= distance in graph) of overlap region
    int overlapSize (int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int) const) &(ThisType::overlapSize)),
        ((int (GridImp::*)(int) const) &(GridImp::overlapSize )));
      return asImp().overlapSize(codim);
    }

    //! return size (= distance in graph) of ghost region
    int ghostSize (int level, int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int,int) const) &(ThisType::ghostSize)),
        ((int (GridImp::*)(int,int) const) &(GridImp:: ghostSize)));
      return asImp().ghostSize(level,codim);
    }

    //! return size (= distance in graph) of ghost region
    int ghostSize (int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int) const) &(ThisType::ghostSize)),
        ((int (GridImp::*)(int) const) &(GridImp:: ghostSize)));
      return asImp().ghostSize(codim);
    }

    //! Iterator to first entity of given codim on level
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::LevelIterator lbegin (int level) const
    {
      typedef typename Codim<cd>::template Partition<pitype>::LevelIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)(int) const) &(ThisType::template lbegin<cd,pitype>)),
        ((ItType (GridImp::*)(int) const) &(GridImp ::template lbegin<cd,pitype>)));
      return asImp().template lbegin<cd,pitype>(level);
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::LevelIterator lend (int level) const
    {
      typedef typename Codim<cd>::template Partition<pitype>::LevelIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)(int) const) &(ThisType::template lend<cd,pitype>)),
        ((ItType (GridImp::*)(int) const) &(GridImp ::template lend<cd,pitype>)));
      return asImp().template lend<cd,pitype>(level);
    }

    //! Iterator to first entity of given codim on level for PartitionType All_Partition
    template<int cd>
    typename Codim<cd>::template Partition<All_Partition>::LevelIterator lbegin (int level) const
    {
      typedef typename Codim<cd>::LevelIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)(int) const) &(ThisType::template lbegin<cd>)),
        ((ItType (GridImp::*)(int) const) &(GridImp ::template lbegin<cd>)));
      return asImp().template lbegin<cd>(level);
    }

    //! one past the end on this level for PartitionType All_Partition
    template<int cd>
    typename Codim<cd>::template Partition<All_Partition>::LevelIterator lend (int level) const
    {
      typedef typename Codim<cd>::LevelIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)(int) const) &(ThisType::template lend<cd>)),
        ((ItType (GridImp::*)(int) const) &(GridImp ::template lend<cd>)));
      return asImp().template lend<cd>(level);
    }

    //! Iterator to first entity of given codim on leaf level of the grid
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::LeafIterator leafbegin () const
    {
      typedef typename Codim<cd>::template Partition<pitype>::LeafIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)() const) &(ThisType::template leafbegin<cd,pitype>)),
        ((ItType (GridImp::*)() const) &(GridImp ::template leafbegin<cd,pitype>)));
      return asImp().template leafbegin<cd,pitype>();
    }

    //! one past the end on the leaf level of this grid
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::LeafIterator leafend () const
    {
      typedef typename Codim<cd>::template Partition<pitype>::LeafIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)() const) &(ThisType::template leafend<cd,pitype>)),
        ((ItType (GridImp::*)() const) &(GridImp ::template leafend<cd,pitype>)));
      return asImp().template leafend<cd,pitype>();
    }

    //! Iterator to first entity of given codim on leaf level of the grid
    template<int cd>
    typename Codim<cd>::template Partition<All_Partition>::LeafIterator leafbegin () const
    {
      typedef typename Codim<cd>::LeafIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)() const) &(ThisType::template leafbegin<cd>)),
        ((ItType (GridImp::*)() const) &(GridImp ::template leafbegin<cd>)));
      return asImp().template leafbegin<cd,All_Partition>();
    }

    //! one past the end on the leaf level of this grid
    template<int cd>
    typename Codim<cd>::template Partition<All_Partition>::LeafIterator leafend () const
    {
      typedef typename Codim<cd>::LeafIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)() const) &(ThisType::template leafend<cd>)),
        ((ItType (GridImp::*)() const) &(GridImp ::template leafend<cd>)));
      return asImp().template leafend<cd,All_Partition>();
    }

    //! return const reference to the grids global id set
    const typename Codim<0>::GlobalIdSet& globalIdSet() const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((const typename Codim<0>::GlobalIdSet& (GridImp::*)() const) &ThisType::globalIdSet),
        ((const typename Codim<0>::GlobalIdSet& (GridImp::*)() const) &GridImp::globalIdSet));
      return asImp().globalIdSet();
    }

    //! return const reference to the grids local id set
    const typename Codim<0>::LocalIdSet& localIdSet() const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((const typename Codim<0>::LocalIdSet& (GridImp::*)() const) &ThisType::localIdSet),
        ((const typename Codim<0>::LocalIdSet& (GridImp::*)() const) &GridImp::localIdSet));
      return asImp().localIdSet();
    }

    //! return const reference to the grids level index set for level level
    const typename Codim<0>::LevelIndexSet& levelIndexSet(int level) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((const typename Codim<0>::LevelIndexSet& (GridImp::*)(int) const) &ThisType::levelIndexSet),
        ((const typename Codim<0>::LevelIndexSet& (GridImp::*)(int) const) &GridImp::levelIndexSet));
      return asImp().levelIndexSet(level);
    }

    //! return const reference to the grids leaf index set
    const typename Codim<0>::LeafIndexSet& leafIndexSet() const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((const typename Codim<0>::LeafIndexSet& (GridImp::*)() const) &ThisType::leafIndexSet),
        ((const typename Codim<0>::LeafIndexSet& (GridImp::*)() const) &GridImp::leafIndexSet));
      return asImp().leafIndexSet();
    }

  private:
    //!  Barton-Nackman trick
    GridImp& asImp () {return static_cast<GridImp &> (*this);}
    //!  Barton-Nackman trick
    const GridImp& asImp () const {return static_cast<const GridImp &>(*this);}
  };


  //************************************************************************
  //
  //  Default Methods of Grid
  //
  //************************************************************************
  //

  /**
     \ingroup GridInterface
     @{

     A Grid is a container of grid entities. Given a dimension dim
     these entities have a codimension codim with 0 <= codim <= dim.

     The Grid is assumed to be hierachically refined and nested. It
     enables iteration over entities of a given level and codimension.

     The grid can be non-matching.

     All information is provided to allocate degrees of freedom in
     appropriate vector data structures (which are not part of this
     module).

     Template class Grid defines a "base class" for all grids.

     \par Classes implementing the Grid Interface
     \li Dune::AlbertaGrid <br>
         <i> Provides the simplicial meshes of the finite element tool box
             ALBERTA (http://www.alberta-fem.de/)
             written by Kunibert Siebert and Alfred Schmidt.</i>
     \li Dune::ALU3dGrid <br>
         <i> 3D grid with support for hexahedrons and tetrahedrons.</i>
     \li Dune::OneDGrid <br>
         <i> Onedimensional adaptive grid</i>
     \li Dune::SGrid <br>
         <i> A structured mesh in d dimensions consisting of "cubes".</i>
     \li Dune::UGGrid <br>
         <i> Provides the meshes of the finite element toolbox UG.
             (http://cox.iwr.uni-heidelberg.de/~ug).</i>
     \li Dune::YaspGrid (Yet Another Structured Parallel Grid) <br>
         <i> Provides a distributed structured cube mesh.</i>
   */
  template<int dim,
      int dimworld,
      class ct,
      class GridFamily>
  class GridDefaultImplementation : public Grid <dim,dimworld,ct,GridFamily>
  {
    typedef typename GridFamily::Traits::Grid GridImp;

  public:
    //! the traits of this class
    typedef typename GridFamily::Traits Traits;

    enum {
      //! \brief The dimension of the grid
      dimension=dim
    };

    enum {
      //! \brief The dimension of the world the grid lives in.
      dimensionworld=dimworld
    };

    //! Define type used for coordinates in grid module
    typedef ct ctype;
    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************
    /** \brief marks an entity for refCount refines.
     *
     * If refCount is negative the entity is coarsened -refCount times
     * \return true if entity was marked, false otherwise
     *
     *  - Note:
     *    default implementation is: return false; for grids with no
     *    adaptation.
     *  - for the grid programmer:
     *    this method is implemented as a template method, because the
     *    Entity type is not defined when the class is instantiated
     *
     *    You won't need this trick in the implementation.
     *    In your implementation you should use it as
     *    \code
     *    bool mark( int refCount,
     *               typename Traits::template Codim<0>::EntityPointer & e ).
     *    \endcode
     *    This template method will vanish due to the inheritance
     *    rules.
     */
    template <class T>
    bool mark( int refCount, T & e )
    {
      IsTrue<Conversion<T, typename Grid<dim,dimworld,ct,GridFamily>::template Codim<0>::EntityPointer>::exists >::yes();
      return false;
    }

    /** \brief Refine all positive marked leaf entities
        coarsen all negative marked entities if possible
        \return true if a least one entity was refined

        - Note: this default implementation always returns false
          so grid with no adaptation doesn't need to implement these methods
     */
    bool adapt ()    { return false; }

    //! returns true, if at least one entity is marked for adaption
    bool preAdapt () { return false; }

    //! clean up some markers
    void postAdapt() {}

    /** \brief ghostSize is zero by default */
    int ghostSize (int level, int codim) const { return 0; }

    /** \brief overlapSize is zero by default */
    int overlapSize (int level, int codim) const { return 0; }

    /** \brief ghostSize is zero by default */
    int ghostSize (int codim) const { return 0; }

    /** \brief overlapSize is zero by default */
    int overlapSize (int codim) const { return 0; }

    /** dummy communicate, doing nothing  */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level) const
    {}

    /** dummy communicate, doing nothing  */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir) const
    {}

  protected:
    //! return real implementation of interface class
    template <class InterfaceType>
    typename InterfaceType :: ImplementationType &
    getRealImplementation (InterfaceType &i) const { return i.getRealImp(); }

    //! return real implementation of interface class
    template <class InterfaceType>
    const typename InterfaceType :: ImplementationType &
    getRealImplementation (const InterfaceType &i) const { return i.getRealImp(); }

  protected:
    //! Barton-Nackman trick
    GridImp& asImp () {return static_cast<GridImp &>(*this);}
    const GridImp& asImp () const {return static_cast<const GridImp &>(*this);}
  };

  /** @} */

  template <int dim, int dimw, class GridImp,
      template<int,int,class> class GeometryImp,
      template<int,int,class> class EntityImp,
      template<int,class> class EntityPointerImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp,
      template<class> class IntersectionIteratorImp,
      template<class> class HierarchicIteratorImp,
      template<int,PartitionIteratorType,class> class LeafIteratorImp,
      class LevelIndexSetImp, class LevelIndexSetTypes, class LeafIndexSetImp, class LeafIndexSetTypes,
      class GlobalIdSetImp, class GIDType, class LocalIdSetImp, class LIDType>
  struct GridTraits
  {
    typedef GridImp Grid;

    typedef Dune::IntersectionIterator<const GridImp, IntersectionIteratorImp> IntersectionIterator;

    typedef Dune::HierarchicIterator<const GridImp, HierarchicIteratorImp> HierarchicIterator;

    template <int cd>
    struct Codim
    {
      // IMPORTANT: Codim<codim>::Geometry == Geometry<dim-codim,dimw>
      typedef Dune::Geometry<dim-cd, dimw, const GridImp, GeometryImp> Geometry;
      typedef Dune::Geometry<dim-cd, dim, const GridImp, GeometryImp> LocalGeometry;
      // we could - if needed - introduce another struct for dimglobal of Geometry
      typedef Dune::Entity<cd, dim, const GridImp, EntityImp> Entity;

      typedef Dune::LevelIterator<cd,All_Partition,const GridImp,LevelIteratorImp> LevelIterator;

      typedef Dune::LeafIterator<cd,All_Partition,const GridImp,LeafIteratorImp> LeafIterator;

      typedef Dune::EntityPointer<const GridImp,EntityPointerImp<cd,const GridImp> > EntityPointer;

      template <PartitionIteratorType pitype>
      struct Partition
      {
        typedef Dune::LevelIterator<cd,pitype,const GridImp,LevelIteratorImp> LevelIterator;
        typedef Dune::LeafIterator<cd,pitype,const GridImp,LeafIteratorImp> LeafIterator;
      };

    };

    typedef IndexSet<const GridImp,LevelIndexSetImp,LevelIndexSetTypes> LevelIndexSet;
    typedef IndexSet<const GridImp,LeafIndexSetImp,LeafIndexSetTypes> LeafIndexSet;
    typedef IdSet<const GridImp,GlobalIdSetImp,GIDType> GlobalIdSet;
    typedef IdSet<const GridImp,LocalIdSetImp,LIDType> LocalIdSet;
  };

  /*! \internal
        Used for grid I/O
   */
  enum GridIdentifier { SGrid_Id, AlbertaGrid_Id , SimpleGrid_Id, UGGrid_Id,
                        YaspGrid_Id , ALU3dGrid_Id, OneDGrid_Id };

  //! provide names for the different grid types
  inline std::string transformToGridName(GridIdentifier type)
  {
    switch(type) {
    case SGrid_Id :
      return "SGrid";
    case AlbertaGrid_Id :
      return "AlbertaGrid";
    case ALU3dGrid_Id :
      return "ALU3dGrid";
    case SimpleGrid_Id :
      return "SimpleGrid";
    case UGGrid_Id :
      return "UGGrid";
    case YaspGrid_Id :
      return "YaspGrid";
    case OneDGrid_Id :
      return "OneDGrid";
    default :
      return "unknown";
    }
  }

  // define of capabilties for the interface class
  namespace Capabilities
  {
    // capabilities for the interface class depend on the implementation
    template< int dim, int dimworld, typename ct, class GridFamily >
    struct hasLeafIterator< GridDefaultImplementation<dim,dimworld,ct,GridFamily> >
    {
      typedef GridDefaultImplementation<dim,dimworld,ct,GridFamily> GridType;
      typedef typename GridType::Traits::Grid GridImp;
      static const bool v = hasLeafIterator<GridImp>::v;
    };

    // capabilities for the interface class depend on the implementation
    template< int dim, int dimworld, typename ct, class GridFamily , int cdim >
    struct hasEntity< GridDefaultImplementation<dim,dimworld,ct,GridFamily>, cdim >
    {
      typedef GridDefaultImplementation<dim,dimworld,ct,GridFamily> GridType;
      typedef typename GridType::Traits::Grid GridImp;
      static const bool v = hasEntity<GridImp,cdim>::v;
    };

  } // end namespace Capabilities

  //! for creation of an engine interface object like Entity or Geometry
  //! one has to derive a class to create the object because the
  //! contructors of the interface object classes are protected
  //! therefore here a generic implementation for this object creation is
  //! provided
  template <class InterfaceType>
  struct MakeableInterfaceObject : public InterfaceType
  {
    typedef typename InterfaceType::ImplementationType ImplementationType;
    //! create interface object by calling the contructor of the base class
    MakeableInterfaceObject(const ImplementationType & realImp) : InterfaceType(realImp) {}
  };
}

#undef CHECK_INTERFACE_IMPLEMENTATION

#include "geometry.hh"
#include "entity.hh"
#include "entitypointer.hh"
#include "leveliterator.hh"
#include "intersectioniterator.hh"
#include "hierarchiciterator.hh"
#include "leafiterator.hh"
#include "indexidset.hh"

inline std::ostream& operator<< (std::ostream& s, Dune::PartitionType t)
{
  s << Dune::PartitionName(t);
  return s;
}

inline std::ostream& operator<< (std::ostream& s, Dune::GridIdentifier t)
{
  s << Dune::transformToGridName(t);
  return s;
}

#endif
