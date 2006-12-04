// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune {

  //***********************************************
  template <>
  inline ALUSimplexGrid<3,3>*
  MacroGrid :: Impl<ALUSimplexGrid<3,3> >::
  generate(MacroGrid& mg,const char* filename, MPICommunicatorType MPICOMM )
  {
    mg.element=Simplex;
    std::string str(filename);

#if ALU3DGRID_PARALLEL
    int myrank;
    MPI_Comm_rank(MPICOMM,&myrank);
    if(myrank <= 0)
    {
#endif
    // note: in parallel only on proc 0 macro grid is generated
    MacroGrid::Impl<ALUSimplexGrid<3,3> >().
    generateAlu3d(mg,filename,str,MPICOMM);
#if ALU3DGRID_PARALLEL
    return new ALUSimplexGrid<3,3>(str.c_str(),MPICOMM);
  }
  else
  {
    return new ALUSimplexGrid<3,3>(MPICOMM);
  }
#else
    return new ALUSimplexGrid<3,3>(str.c_str());
#endif
}

template <>
inline ALUCubeGrid<3,3>*
MacroGrid :: Impl<ALUCubeGrid<3,3> >::generate
  (MacroGrid& mg,const char* filename, MPICommunicatorType MPICOMM )
{
  mg.element=Cube;
  std::string str(filename);

#if ALU3DGRID_PARALLEL
  int myrank;
  MPI_Comm_rank(MPICOMM,&myrank);
  if(myrank <= 0)
  {
#endif
  // note: in parallel only on proc 0 macro grid is generated
  MacroGrid::Impl<ALUCubeGrid<3,3> >().
  generateAlu3d(mg,filename,str,MPICOMM);
#if ALU3DGRID_PARALLEL
  return new ALUCubeGrid<3,3>(str.c_str(),MPICOMM);
}
else
{
  return new ALUCubeGrid<3,3>(MPICOMM);
}
#else
  return new ALUCubeGrid<3,3>(str.c_str());
#endif
}

template <>
inline ALUSimplexGrid<2,2>*
MacroGrid :: Impl<ALUSimplexGrid<2,2> >::
generate(MacroGrid& mg,const char* filename, MPICommunicatorType MPICOMM )
{
  mg.element=Simplex;
  std::string str(filename);
  MacroGrid::Impl<ALUSimplexGrid<2,2> >().
  generateAlu3d(mg,filename,str,MPICOMM);
  return new ALUSimplexGrid<2,2>(str.c_str());
}

/* needs newer version of alulib
   template <>
   inline ALUConformGrid<2,2>*
   MacroGrid :: Impl<ALUConformGrid<2,2> >::generate
   (MacroGrid& mg,const char* filename, MPICommunicatorType MPICOMM ) {
   mg.element=Simplex;
   std::string str(filename);
   MacroGrid::Impl<ALUConfromGrid<2,2> >().
    generateAlu3d(mg,filename,str,MPICOMM);
   return new ALUConfromGrid<2,2>(str.c_str());
   }
   template <>
   inline void
   MacroGrid :: Impl<ALUConformGrid<2,2> > :: generateAlu3d
   (MacroGrid& mg,const char* filename, std::string& str, MPICommunicatorType MPICOMM ) {
   MacroGrid::Impl<ALUSimplexGrid<2,2> >().
    generateAlu3d(mg,filename,str,MPICOMM);
   }
 */
template <int dim,int dimworld>
inline void
MacroGrid :: Impl<ALUSimplexGrid<dim,dimworld> > ::
generateAlu3d(MacroGrid& mg,const char* filename, std::string& str, MPICommunicatorType MPICOMM )
{
  std::ifstream gridin(filename);
  if(mg.readDuneGrid(gridin))
  {
    if (mg.dimw != dimworld)
    {
      DUNE_THROW(DGFException,
                 "Macrofile " << filename << " is for dimension " << mg.dimw
                              << " and connot be used to initialize an ALUGrid of dimension "
                              << dimworld);
    }
    mg.setOrientation(dimworld-1,dimworld);
    // mg.setRefinement(dimworld-1,dimworld,-1,-1);
    str+=".ALUgrid";
    std::ofstream out(str.c_str());
    mg.writeAlu(out);
  }
}
template <int dim,int dimworld>
inline void
MacroGrid :: Impl<ALUCubeGrid<dim,dimworld> > :: generateAlu3d
  (MacroGrid& mg,const char* filename, std::string& str, MPICommunicatorType MPICOMM )
{
  std::ifstream gridin(filename);
  if(mg.readDuneGrid(gridin))
  {
    if (mg.dimw != dimworld)
    {
      DUNE_THROW(DGFException,
                 "Macrofile " << filename << " is for dimension " << mg.dimw
                              << " and connot be used to initialize an ALUGrid of dimension "
                              << dimworld);
    }
    str+=".ALUgrid";
    std::ofstream out(str.c_str());
    mg.writeAlu(out);
  }
}

} // end namespace Dune
