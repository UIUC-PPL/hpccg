#if !defined(CHARM_HPCCG)
#define CHARM_HPCCG

#include "HPC_Sparse_Matrix.hpp"
#include <string>
#include <pup_stl.h>
#include "charmHpccg.decl.h"

/*readonly*/ extern int numChares;

class charmHpccg : public CBase_charmHpccg {
public:
  charmHpccg() { }
  charmHpccg(CkMigrateMessage*) { }
  void generateMatrix(int nx, int ny, int nz);
  void readMatrix(std::string fileName);
  void findExternals() { }
  void needElement(CkArrayIndex1D idx, int row, int col)  { }
protected:
  HPC_Sparse_Matrix* A;
  double *x, *b, *xexact;
};

class charmMain : public CBase_charmMain {
public:
  charmMain(CkArgMsg* msg);
  void matrixReady();
  void foundExternals();
private:
  CProxy_charmHpccg array;
};

#endif /*CHARM_HPCCG*/
