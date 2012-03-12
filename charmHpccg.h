#if !defined(CHARM_HPCCG)
#define CHARM_HPCCG

#include "HPC_Sparse_Matrix.hpp"
#include <string>
#include <map>
#include <vector>
#include <pup_stl.h>
#include "charmHpccg.decl.h"

/*readonly*/ extern int numChares;

class charmHpccg : public CBase_charmHpccg {
public:
  charmHpccg() : xMessagesExpected(0) { }
  charmHpccg(CkMigrateMessage*) { }
  void generateMatrix(int nx, int ny, int nz);
  void readMatrix(std::string fileName);
  void findExternals();
  void needXElements(int idx, std::vector<int> row);
protected:
  HPC_Sparse_Matrix* A;
  double *x, *b, *xexact;
  int xMessagesExpected;
  std::map<int, std::vector<int> > xToReceive, xToSend; // chare index -> rows of x
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
