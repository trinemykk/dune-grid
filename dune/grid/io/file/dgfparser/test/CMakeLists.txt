dune_add_test(NAME test-dgf-yasp
              SOURCES test-dgf-yasp.cc
              LINK_LIBRARIES dunegrid
              COMPILE_DEFINITIONS DUNE_GRID_EXAMPLE_GRIDS_PATH=\"${PROJECT_SOURCE_DIR}/doc/grids/\"
             )

dune_add_test(NAME test-dgf-yasp-offset
              SOURCES test-dgf-yasp.cc
              LINK_LIBRARIES dunegrid
              COMPILE_DEFINITIONS DUNE_GRID_EXAMPLE_GRIDS_PATH=\"${PROJECT_SOURCE_DIR}/doc/grids/\"
                                  TESTCOORDINATES
              CMD_ARGS ${PROJECT_SOURCE_DIR}/doc/grids/dgf/test2d_offset.dgf)

dune_add_test(NAME test-dgf-oned
              SOURCES test-dgf-oned.cc
              LINK_LIBRARIES dunegrid
              COMPILE_DEFINITIONS DUNE_GRID_EXAMPLE_GRIDS_PATH=\"${PROJECT_SOURCE_DIR}/doc/grids/\"
             )

if(ALBERTA_FOUND)
  add_executable(test-dgf-alberta test-dgf-alberta.cc)
  add_dune_alberta_flags(test-dgf-alberta GRIDDIM 2)
  target_compile_definitions(test-dgf-alberta PUBLIC
    DUNE_GRID_EXAMPLE_GRIDS_PATH=\"${PROJECT_SOURCE_DIR}/doc/grids/\")
  dune_add_test(TARGET test-dgf-alberta)
endif(ALBERTA_FOUND)

dune_add_test(NAME test-dgf-ug
              SOURCES test-dgf-ug.cc
              COMPILE_DEFINITIONS DUNE_GRID_EXAMPLE_GRIDS_PATH=\"${PROJECT_SOURCE_DIR}/doc/grids/\"
              CMAKE_GUARD dune-uggrid_FOUND)
