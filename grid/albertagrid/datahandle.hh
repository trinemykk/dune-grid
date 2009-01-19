// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTAGRIDDATAHANDLE_HH
#define DUNE_ALBERTAGRIDDATAHANDLE_HH

#if HAVE_ALBERTA

#include <iostream>

#include <dune/grid/common/grid.hh>

#include <dune/grid/albertagrid/misc.hh>
#include <dune/grid/albertagrid/albertaheader.hh>
#include <dune/grid/albertagrid/albertaextra.hh>
#include <dune/grid/albertagrid/elementinfo.hh>

namespace Dune
{

  namespace Alberta
  {

    template< class Grid, class RestrictProlongOperator >
    class AdaptRestrictProlongHandler
    {
      static const int dimension = Grid::dimension;

      typedef typename Grid::template Codim< 0 >::Entity Entity;
      typedef Dune::MakeableInterfaceObject< Entity > EntityObject;
      typedef typename EntityObject::ImplementationType EntityImp;

      typedef Alberta::ElementInfo< dimension > ElementInfo;
      typedef Alberta::Patch< dimension > Patch;

      Grid &grid_;
      RestrictProlongOperator &rpOp_;
      EntityObject father_;

    public:
      AdaptRestrictProlongHandler ( Grid &grid, RestrictProlongOperator &rpOp )
        : grid_( grid ),
          rpOp_( rpOp ),
          father_( EntityImp( grid_ ) )
      {}

      void restrictLocal ( const Patch &patch, int i )
      {
        ElementInfo fatherInfo = patch.elementInfo( i, grid_.levelProvider() );
        Grid::getRealImplementation( father_ ).setElement( fatherInfo, 0 );
        rpOp_.restrictLocal( (const Entity &)father_ );
      }

      void prolongLocal ( const Patch &patch, int i )
      {
        ElementInfo fatherInfo = patch.elementInfo( i, grid_.levelProvider() );
        Grid::getRealImplementation( father_ ).setElement( fatherInfo, 0 );
        rpOp_.prolongLocal( (const Entity &)father_ );
      }
    };

  }

}

#endif // #if HAVE_ALBERTA

#endif
