// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_YASPGRIDENTITY_HH
#define DUNE_GRID_YASPGRIDENTITY_HH

/** \file
 * \brief the YaspEntity class and its specializations
 *
   YaspEntity realizes the concept a mesh entity.

   We have specializations for
    - codim==0 (elements), because they have an extended interface
    - codim==dim (vertices), because they use a special constructor
      of the AxisAlignedCubeGeometry and are the only entities in a
      globally refined grid, that may also exist on another level.

   The general version implements all other codimensions.
 */
//========================================================================




namespace Dune {

  namespace Yasp {

#ifndef DOXYGEN

    // table for quick evaluation of binomial coefficients
    template<int n>
    struct BinomialTable
    {
      static void init()
      {
        if (_initialized)
          return;
        int offset = 0;
        for (int d = 0; d <= n; ++d)
          {
            _offsets[d] = offset;
            for (int c = 0; c <= d; ++c, ++offset)
              _values[offset] = binomial(d,c);
          }
        _initialized = true;
      }

      // evaluation - note that in general d!=n, n is only the
      // maximum value of d (in our case dimworld)
      static int evaluate(int d, int c)
      {
        return _values[_offsets[d] + c];
      }

    private:
      // prevent construction
      BinomialTable();

      static bool _initialized;
      static std::array<int,(n+1)*(n+2)/2> _values;
      static std::array<int,n+1> _offsets;

    public:

      // the actual implementation
      static int binomial(int d, int c)
      {
        long binomial=1;
        for (int i=d-c+1; i<=d; i++)
          binomial *= i;
        for (long i=2; i<=c; i++)
          binomial /= i;
        return binomial;
      }
    };

    template<int n>
    bool BinomialTable<n>::_initialized = false;
    template<int n>
    std::array<int,(n+1)*(n+2)/2> BinomialTable<n>::_values;
    template<int n>
    std::array<int,n+1> BinomialTable<n>::_offsets;

    /** \returns number of subentities of given codim in a cube of dimension dim
     *  \tparam dimworld the maximum dimension the table holds entries for
     *  \param d the dimension of the cube
     *  \param c the codimension we are interested in
     *  That number is d choose c times 2^c.
     */
    template<int dimworld>
    int subEnt(int d, int c)
    {
      return (d < c ? 0 : BinomialTable<dimworld>::evaluate(d,c) << c);
    }

    // Make a table mapping all subentities of a codim 0 entity to a value.
    // F is the functor to be evaluated.
    template<typename F, int dim>
    struct EntityShiftTable
    {

      typedef std::bitset<dim> value_type;

      static void init()
      {
        if (_initialized)
          return;
        F f;
        int offset = 0;
        for (int codim = 0; codim <= dim; ++codim)
          {
            _offsets[codim] = offset;
            for (int i = 0; i < subEnt<dim>(dim,codim); ++i, ++offset)
              _values[offset] = static_cast<unsigned char>(f(i,codim).to_ulong());
          }
        _initialized = true;
      }

      static value_type evaluate(int i, int codim)
      {
        return {_values[_offsets[codim] + i]};
      }

    private:

      // prevent construction
      EntityShiftTable();

      static bool _initialized;
      static std::array<int,dim+1> _offsets;
      static std::array<unsigned char,StaticPower<3,dim>::power> _values;

    };

    template<typename F, int dim>
    bool EntityShiftTable<F,dim>::_initialized = false;
    template<typename F, int dim>
    std::array<int,dim+1> EntityShiftTable<F,dim>::_offsets;
    template<typename F, int dim>
    std::array<unsigned char,StaticPower<3,dim>::power> EntityShiftTable<F,dim>::_values;

    // functor for doing the actual entity shift calculation
    template<int dim>
    struct calculate_entity_shift
    {
      calculate_entity_shift()
      {
        BinomialTable<dim>::init();
      }

      std::bitset<dim> operator()(int index, int cc) const
      {
        std::bitset<dim> result(0ull);
        for (int d = dim; d>0; d--)
          {
            if (cc == d)
              return result;
            if (index < subEnt<dim>(d-1,cc))
              result[d-1]=true;
            else
              {
                index = (index - subEnt<dim>(d-1, cc)) % subEnt<dim>(d-1,cc-1);
                cc--;
              }
          }
        return result;
      }
    };

    /** \returns a shift vector as used by YGridComponent
     * \param index subentity index
     * \param cc the codimension
     * This maps a subentity, given by its codimension and index, to
     * a bitset which specifies the unit vectors which span the entity.
     * The implementation unravels the construction of the generic
     * reference elements.
     */
    template<int dim>
    std::bitset<dim> entityShift(int index, int cc)
    {
      return EntityShiftTable<calculate_entity_shift<dim>,dim>::evaluate(index,cc);
    }

    // functor for doing the actual entity move calculation
    template<int dim>
    struct calculate_entity_move
    {

      calculate_entity_move()
      {
        BinomialTable<dim>::init();
      }

      std::bitset<dim> operator()(int index, int cc) const
      {
        std::bitset<dim> result(0ull);
        for (int d = dim; d>0; d--)
          {
            if (d == cc)
              {
                result[d-1] = index & (1<<(d-1));
                index &= ~(1<<(d-1));
              }
            if (index >= subEnt<dim>(d-1,cc))
              {
                if ((index - subEnt<dim>(d-1,cc)) / subEnt<dim>(d-1,cc-1) == 1)
                  {
                    result[d-1] = true;
                  }
                index = (index - subEnt<dim>(d-1, cc)) % subEnt<dim>(d-1,cc-1);
                cc--;
              }
          }
        return result;
      }

    };

    /** \returns a bitset telling in which direction to move a cell to get
     *    the cell a given entity is living on.
     *  \param index subentity index
     *  \param cc the codimension
     *  In Yasp, all entities live on a cell. Its those that lie on the lower
     *  / left / ... part of the cell.
     */
    template<int dim>
    std::bitset<dim> entityMove(int index, int cc)
    {
      return EntityShiftTable<calculate_entity_move<dim>,dim>::evaluate(index,cc);
    }

#endif //DOXYGEN

  } // namespace Yasp.

  template<int codim, int dim, class GridImp>
  class YaspEntity
    :  public EntityDefaultImplementation <codim,dim,GridImp,YaspEntity>
  {

    template<int, PartitionIteratorType, typename>
    friend class YaspLevelIterator;

  public:
    typedef typename GridImp::ctype ctype;

    typedef typename GridImp::template Codim<codim>::Geometry Geometry;
    typedef typename GridImp::Traits::template Codim<codim>::GeometryImpl GeometryImpl;

    typedef typename GridImp::template Codim<codim>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<codim>::EntitySeed EntitySeed;

    //! level of this element
    int level () const
    {
      return _g->level();
    }

    /** \brief Return the entity seed which contains sufficient information
     *  to generate the entity again and uses as little memory as possible
     */
    EntitySeed seed() const
    {
      return EntitySeed(YaspEntitySeed<codim,GridImp>(_g->level(), _it.coord(), _it.which()));
    }

    //! geometry of this entity
    Geometry geometry () const
    {
      GeometryImpl _geometry(_it.lowerleft(),_it.upperright(),_it.shift());
      return Geometry(_geometry);
    }

    //! return partition type attribute
    PartitionType partitionType () const
    {
      if (_g->interior[codim].inside(_it.coord(),_it.shift()))
        return InteriorEntity;
      if (_g->interiorborder[codim].inside(_it.coord(),_it.shift()))
        return BorderEntity;
      if (_g->overlap[codim].inside(_it.coord(),_it.shift()))
        return OverlapEntity;
      if (_g->overlapfront[codim].inside(_it.coord(),_it.shift()))
        return FrontEntity;
      return GhostEntity;
    }

    typedef typename GridImp::YGridLevelIterator YGLI;
    typedef typename GridImp::YGrid::Iterator I;
    YaspEntity ()
    {}

    YaspEntity (const YGLI& g, const I& it)
      : _it(it), _g(g)
    {}

// skip this constructor for GCC 4.4, which has a number of nasty bugs in its rvalue reference support
// As this behavior is hard to trigger in small configuration tests and because we'll probably drop GCC 4.4
// after the next release anyway, I hacked in this hardcoded check for the compiler version
#if not (defined(__GNUC__) && (__GNUC__ < 5) && (__GNUC_MINOR__ < 5))

    YaspEntity (YGLI&& g, const I&& it)
      : _it(std::move(it)), _g(std::move(g))
    {}

#endif

    //! Return true when two iterators over the same grid are equal (!).
    bool equals (const YaspEntity& e) const
    {
      return _it == e._it && _g == e._g;
    }

    // IndexSets needs access to the private index methods
    friend class Dune::YaspIndexSet<GridImp,true>;
    friend class Dune::YaspIndexSet<GridImp,false>;
    friend class Dune::YaspGlobalIdSet<GridImp>;
    typedef typename GridImp::PersistentIndexType PersistentIndexType;

    //! globally unique, persistent index
    PersistentIndexType persistentIndex () const
    {
      // get size of global grid (in elements)
      Dune::array<int,dim> size;

      for (int i=0; i<dim; i++)
      {
        // correct size according to shift
        size[i] = _g->mg->levelSize(_g->level(), i);
        if (!_it.shift(i))
          size[i]++;
      }

      // encode codim
      PersistentIndexType id(_it.shift().to_ulong());

      // encode level
      id = id << yaspgrid_level_bits;
      id = id+PersistentIndexType(_g->level());

      // encode coordinates
      for (int i=dim-1; i>=0; i--)
      {
        id = id << yaspgrid_dim_bits;
        id = id+PersistentIndexType(_it.coord(i));
      }

      return id;
    }

    //! consecutive, codim-wise, level-wise index
    int compressedIndex () const
    {
      return _it.superindex();
    }

    //! subentity compressed index
    int subCompressedIndex (int i, unsigned int cc) const
    {
      // get the shift of the entity and the subentity
      // the subentity shift is only available in the space spanned by the entity
      std::bitset<dim> ent_shift = _it.shift();
      std::bitset<dim-codim> subent_shift = Dune::Yasp::entityShift<dim-codim>(i,cc);
      std::bitset<dim-codim> subent_move = Dune::Yasp::entityMove<dim-codim>(i,cc);
      // combine the shifts to get the global shift of the subentity
      std::bitset<dim> shift,move;
      for (int i=0,j=0; i<dim; i++)
        if (ent_shift[i])
        {
          shift[i] = subent_shift[j];
          move[i] = subent_move[j];
          j++;
        }

      Dune::array<int, dim> size = _g->mg->levelSize(_g->level());
      Dune::array<int, dim> coord = _it.coord();
      for (int j=0; j<dim; j++)
      {
        if (!shift[j])
          size[j]++;
        if (move[j])
          coord[j]++;
      }

      int which = _g->overlapfront[cc].shiftmapping(shift);
      return _g->overlapfront[cc].superindex(coord,which);
    }
    public:
    const I& transformingsubiterator() const { return _it; }
    const YGLI& gridlevel() const { return _g; }
    I& transformingsubiterator() { return _it; }
    YGLI& gridlevel() { return _g; }
    const GridImp * yaspgrid() const { return _g->mg; }
    protected:
    I _it;               // position in the grid level
    YGLI _g;               // access to grid level
  };


  // specialization for codim=0
  template<int dim, class GridImp>
  class YaspEntity<0,dim,GridImp>
    : public EntityDefaultImplementation <0,dim,GridImp,YaspEntity>
  {
    enum { dimworld = GridImp::dimensionworld };

    typedef typename GridImp::Traits::template Codim< 0 >::GeometryImpl GeometryImpl;

    template<int, PartitionIteratorType, typename>
    friend class YaspLevelIterator;

    template<typename>
    friend class YaspHierarchicIterator;

  public:
    typedef typename GridImp::ctype ctype;

    typedef typename GridImp::YGridLevelIterator YGLI;
    typedef typename GridImp::YGrid::Iterator I;

    typedef typename GridImp::template Codim< 0 >::Geometry Geometry;
    typedef typename GridImp::template Codim< 0 >::LocalGeometry LocalGeometry;

    template <int cd>
    struct Codim
    {
      typedef typename GridImp::template Codim<cd>::EntityPointer EntityPointer;
      typedef typename GridImp::template Codim<cd>::Entity Entity;
    };

    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::template Codim<0>::EntitySeed EntitySeed;
    typedef typename GridImp::LevelIntersectionIterator IntersectionIterator;
    typedef typename GridImp::LevelIntersectionIterator LevelIntersectionIterator;
    typedef typename GridImp::LeafIntersectionIterator LeafIntersectionIterator;
    typedef typename GridImp::HierarchicIterator HierarchicIterator;

    //! define the type used for persisitent indices
    typedef typename GridImp::PersistentIndexType PersistentIndexType;

    //! define type used for coordinates in grid module
    typedef typename GridImp::YGrid::iTupel iTupel;

    // constructor
    YaspEntity ()
    {}

    YaspEntity (const YGLI& g, const I& it)
      : _it(it), _g(g)
    {}

    YaspEntity (const YGLI& g, I&& it)
      : _it(std::move(it)), _g(g)
    {}

// skip this constructor for GCC 4.4, which has a number of nasty bugs in its rvalue reference support
// As this behavior is hard to trigger in small configuration tests and because we'll probably drop GCC 4.4
// after the next release anyway, I hacked in this hardcoded check for the compiler version
#if not (defined(__GNUC__) && (__GNUC__ < 5) && (__GNUC_MINOR__ < 5))

    YaspEntity (YGLI&& g, I&& it)
      : _it(std::move(it)), _g(std::move(g))
    {}

#endif

    //! Return true when two iterators over the same grid are equal (!).
    bool equals (const YaspEntity& e) const
    {
      return _it == e._it && _g == e._g;
    }

    //! level of this element
    int level () const { return _g->level(); }

    /** \brief Return the entity seed which contains sufficient information
     *  to generate the entity again and uses as little memory as possible
     */
    EntitySeed seed () const {
      return EntitySeed(YaspEntitySeed<0,GridImp>(_g->level(), _it.coord()));
    }

    //! return partition type attribute
    PartitionType partitionType () const
    {
      if (_g->interior[0].inside(_it.coord(),_it.shift()))
        return InteriorEntity;
      if (_g->overlap[0].inside(_it.coord(),_it.shift()))
        return OverlapEntity;
      DUNE_THROW(GridError, "Impossible GhostEntity");
      return GhostEntity;
    }

    //! geometry of this entity
    Geometry geometry () const {
      // the element geometry
      GeometryImpl _geometry(_it.lowerleft(),_it.upperright());
      return Geometry( _geometry );
    }

    /*! Return number of subentities with codimension cc.
     *
     * That number is (dim over (dim-codim)) times 2^codim
     */
    template<int cc> int count () const
    {
      return Dune::Yasp::subEnt<dim>(dim,cc);
    }

    /*! Return number of subentities with codimension cc.
     *
     * That number is (dim over (dim-codim)) times 2^codim
     */
    unsigned int subEntities (unsigned int codim) const
    {
      return Dune::Yasp::subEnt<dim>(dim,codim);
    }

    /*! Intra-element access to subentities of codimension cc > codim.
     */
    template<int cc>
    typename Codim<cc>::Entity subEntity (int i) const
    {
      // calculate move bitset
      std::bitset<dim> move = Dune::Yasp::entityMove<dim>(i,cc);

      // get the coordinate and modify it
      iTupel coord = _it.coord();
      for (int j=0; j<dim; j++)
        if (move[j])
          coord[j]++;

      int which = _g->overlapfront[cc].shiftmapping(Dune::Yasp::entityShift<dim>(i,cc));
      return typename Codim<cc>::Entity(YaspEntity<cc,GridImp::dimension,GridImp>(_g,_g->overlapfront[cc].begin(coord, which)));
    }

    //! Inter-level access to father element on coarser grid. Assumes that meshes are nested.
    Entity father () const
    {
      // check if coarse level exists
      if (_g->level()<=0)
        DUNE_THROW(GridError, "tried to call father on level 0");

      // yes, get iterator to it
      YGLI cg(_g);
      --cg;

      // coordinates of the cell
      iTupel coord = _it.coord();

      // get coordinates on next coarser level
      for (int k=0; k<dim; k++) coord[k] = coord[k]/2;

      return Entity(YaspEntity<0,GridImp::dimension,GridImp>(cg,cg->overlap[0].begin(coord)));
    }

    //! returns true if father entity exists
    bool hasFather () const
    {
      return (_g->level()>0);
    }

    /*! Location of this element relative to the reference element of its father
     */
    LocalGeometry geometryInFather () const
    {
      // configure one of the 2^dim transformations
      FieldVector<ctype,dim> ll(0.0),ur(0.5);

      for (int k=0; k<dim; k++)
      {
        if (_it.coord(k)%2)
        {
          ll[k] = 0.5;
          ur[k] = 1.0;
        }
      }

      return LocalGeometry( YaspGeometry<dim,dim,GridImp>(ll,ur) );
    }

    const I& transformingsubiterator () const { return _it; }
    const YGLI& gridlevel () const { return _g; }
    I& transformingsubiterator() { return _it; }
    YGLI& gridlevel() { return _g; }
    const GridImp* yaspgrid () const { return _g->mg; }

    bool isLeaf() const
    {
      return (_g->level() == yaspgrid()->maxLevel());
    }

    /**\brief Returns true, if the entity has been created during the last call to adapt()
     */
    bool isNew () const { return yaspgrid()->adaptRefCount > 0 && yaspgrid()->maxLevel() < _g->level() + yaspgrid()->adaptRefCount; }

    /**\brief Returns true, if entity might disappear during the next call to adapt()
     */
    bool mightVanish () const { return false; }

    //! returns intersection iterator for first intersection
    IntersectionIterator ibegin () const
    {
      return YaspIntersectionIterator<GridImp>(*this,false);
    }

    //! returns intersection iterator for first intersection
    LeafIntersectionIterator ileafbegin () const
    {
      // only if entity is leaf this iterator delivers intersections
      return YaspIntersectionIterator<GridImp>(*this, ! isLeaf() );
    }

    //! returns intersection iterator for first intersection
    LevelIntersectionIterator ilevelbegin () const
    {
      return ibegin();
    }

    //! Reference to one past the last neighbor
    IntersectionIterator iend () const
    {
      return YaspIntersectionIterator<GridImp>(*this,true);
    }

    //! Reference to one past the last neighbor
    LeafIntersectionIterator ileafend () const
    {
      return iend();
    }

    //! Reference to one past the last neighbor
    LevelIntersectionIterator ilevelend () const
    {
      return iend();
    }

    /*! Inter-level access to son elements on higher levels<=maxlevel.
          This is provided for sparsely stored nested unstructured meshes.
          Returns iterator to first son.
     */
    HierarchicIterator hbegin (int maxlevel) const
    {
      return YaspHierarchicIterator<GridImp>(_g,_it,maxlevel);
    }

    //! Returns iterator to one past the last son
    HierarchicIterator hend (int maxlevel) const
    {
      return YaspHierarchicIterator<GridImp>(_g,_it,_g->level());
    }

  private:
    // IndexSets needs access to the private index methods
    friend class Dune::YaspIndexSet<GridImp,true>;
    friend class Dune::YaspIndexSet<GridImp,false>;
    friend class Dune::YaspGlobalIdSet<GridImp>;

    //! globally unique, persistent index
    PersistentIndexType persistentIndex () const
    {
      // get size of global grid
      const iTupel& size =  _g->mg->levelSize(_g->level());

      // encode codim
      PersistentIndexType id(_it.shift().to_ulong());

      // encode level
      id = id << yaspgrid_level_bits;
      id = id+PersistentIndexType(_g->level());


      // encode coordinates
      for (int i=dim-1; i>=0; i--)
      {
        id = id << yaspgrid_dim_bits;
        id = id+PersistentIndexType(_it.coord(i));
      }

      return id;
    }

    //! consecutive, codim-wise, level-wise index
    int compressedIndex () const
    {
      return _it.superindex();
    }

    //! subentity persistent index
    PersistentIndexType subPersistentIndex (int i, int cc) const
    {
      // calculate shift and move bitsets
      std::bitset<dim> shift = Dune::Yasp::entityShift<dim>(i,cc);
      std::bitset<dim> move = Dune::Yasp::entityMove<dim>(i,cc);

      int trailing = (cc == dim) ? 1000 : 0;

      Dune::array<int,dim> size = _g->mg->levelSize(_g->level());
      Dune::array<int, dim> coord = _it.coord();
      for (int j=0; j<dim; j++)
      {
        // correct size according to shift
        if (!shift[j])
          size[j]++;

        // move the coordinates to the cell on which the entity lives
        if (move[j])
          coord[j]++;
      }

      for (int j=0; j<dim; j++)
      {
        // in the codim==dim case, count trailing zeroes.
        if (cc == dim)
        {
          int zeroes = 0;
          for (int k=0; k<_g->level(); k++)
            if (coord[j] & (1<<k))
              break;
            else
              zeroes++;
          trailing = std::min(trailing,zeroes);
        }
      }

      // encode codim
      PersistentIndexType id(shift.to_ulong());

      // encode level
      id = id << yaspgrid_level_bits;
      id = id+PersistentIndexType(_g->level()-trailing);

      // encode coordinates
      for (int j=dim-1; j>=0; j--)
      {
        id = id << yaspgrid_dim_bits;
        id = id+PersistentIndexType(coord[j]>>trailing);
      }

      return id;
    }

    //! subentity compressed index
    int subCompressedIndex (int i, int cc) const
    {
      // get shift and move of the subentity in question
      std::bitset<dim> shift = Dune::Yasp::entityShift<dim>(i,cc);
      std::bitset<dim> move = Dune::Yasp::entityMove<dim>(i,cc);

      Dune::array<int,dim> size = _g->mg->levelSize(_g->level());
      Dune::array<int, dim> coord = _it.coord();
      for (int j=0; j<dim; j++)
      {

        size[j] += !shift[j];
        coord[j] += move[j];
      }

      int which = _g->overlapfront[cc].shiftmapping(shift);
      return _g->overlapfront[cc].superindex(coord,which);
    }

    I _it;         // position in the grid level
    YGLI _g;         // access to grid level
  };


  // specialization for codim=dim (vertex)
  template<int dim, class GridImp>
  class YaspEntity<dim,dim,GridImp>
    : public EntityDefaultImplementation <dim,dim,GridImp,YaspEntity>
  {
    enum { dimworld = GridImp::dimensionworld };

    template<int, PartitionIteratorType, typename>
    friend class YaspLevelIterator;

    typedef typename GridImp::Traits::template Codim<dim>::GeometryImpl GeometryImpl;

  public:
    typedef typename GridImp::ctype ctype;

    typedef typename GridImp::YGridLevelIterator YGLI;
    typedef typename GridImp::YGrid::Iterator I;

    typedef typename GridImp::template Codim<dim>::Geometry Geometry;

    template <int cd>
    struct Codim
    {
      typedef typename GridImp::template Codim<cd>::EntityPointer EntityPointer;
    };

    typedef typename GridImp::template Codim<dim>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<dim>::EntitySeed EntitySeed;

    //! define the type used for persisitent indices
    typedef typename GridImp::PersistentIndexType PersistentIndexType;

    //! define type used for coordinates in grid module
    typedef typename GridImp::YGrid::iTupel iTupel;

    // constructor
    YaspEntity ()
    {}

    YaspEntity (const YGLI& g, const I& it)
      : _it(it), _g(g)
    {}

// skip this constructor for GCC 4.4, which has a number of nasty bugs in its rvalue reference support
// As this behavior is hard to trigger in small configuration tests and because we'll probably drop GCC 4.4
// after the next release anyway, I hacked in this hardcoded check for the compiler version
#if not (defined(__GNUC__) && (__GNUC__ < 5) && (__GNUC_MINOR__ < 5))

    YaspEntity (YGLI&& g, I&& it)
      : _it(std::move(it)), _g(std::move(g))
    {}

#endif

    //! Return true when two iterators over the same grid are equal (!).
    bool equals (const YaspEntity& e) const
    {
      return _it == e._it && _g == e._g;
    }

    //! level of this element
    int level () const {return _g->level();}

    /** \brief Return the entity seed which contains sufficient information
     *  to generate the entity again and uses as little memory as possible
     */
    EntitySeed seed () const {
      return EntitySeed(YaspEntitySeed<dim,GridImp>(_g->level(), _it.coord(), _it.which()));
    }

    //! geometry of this entity
    Geometry geometry () const {
      GeometryImpl _geometry((_it).lowerleft());
      return Geometry( _geometry );
    }

    //! return partition type attribute
    PartitionType partitionType () const
    {
      if (_g->interior[dim].inside(_it.coord(),_it.shift()))
        return InteriorEntity;
      if (_g->interiorborder[dim].inside(_it.coord(),_it.shift()))
        return BorderEntity;
      if (_g->overlap[dim].inside(_it.coord(),_it.shift()))
        return OverlapEntity;
      if (_g->overlapfront[dim].inside(_it.coord(),_it.shift()))
        return FrontEntity;
      return GhostEntity;
    }

    //! subentity compressed index simply returns compressedIndex
    int subCompressedIndex (int, unsigned int ) const
    {
      return compressedIndex();
    }

  private:
    // IndexSets needs access to the private index methods
    friend class Dune::YaspIndexSet<GridImp,true>;
    friend class Dune::YaspIndexSet<GridImp,false>;
    friend class Dune::YaspGlobalIdSet<GridImp>;

    //! globally unique, persistent index
    PersistentIndexType persistentIndex () const
    {
      // get coordinate and size of global grid
      iTupel size = _g->mg->levelSize(_g->level());

      for (int i=0; i<dim; i++)
      {
        // we have vertices, add 1 size to all directions
        size[i]++;
      }

      // determine min number of trailing zeroes
      int trailing = 1000;
      for (int i=0; i<dim; i++)
      {
        // count trailing zeros
        int zeros = 0;
        for (int j=0; j<_g->level(); j++)
          if (_it.coord(i)&(1<<j))
            break;
          else
            zeros++;
        trailing = std::min(trailing,zeros);
      }

      // determine the level of this vertex
      int level = _g->level()-trailing;

      // encode codim: shift vector of vertices is 0.
      PersistentIndexType id(0);

      // encode level
      id = id << yaspgrid_level_bits;
      id = id+PersistentIndexType(level);

      // encode coordinates
      for (int i=dim-1; i>=0; i--)
      {
        id = id << yaspgrid_dim_bits;
        id = id+PersistentIndexType(_it.coord(i)>>trailing);
      }

      return id;
    }

    //! consecutive, codim-wise, level-wise index
    int compressedIndex () const { return _it.superindex();}

  public:
    const I& transformingsubiterator() const { return _it; }
    const YGLI& gridlevel() const { return _g; }
    I& transformingsubiterator() { return _it; }
    YGLI& gridlevel() { return _g; }

    const GridImp * yaspgrid() const { return _g->mg; }
  protected:
    I _it;               // position in the grid level
    YGLI _g;               // access to grid level
  };

}   // namespace Dune

#endif  // DUNE_GRID_YASPGRIDENTITY_HH
