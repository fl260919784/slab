# slab
基于tair的mdb_instance扣取的slab内存分配器



g++ -g -std=c++0x -c  simplePagemgrImpl.cpp -DDEBUG
g++ -g -std=c++0x -c  staticPagemgrImpl.cpp -DDEBUG
g++ -g -std=c++0x -c  simpleSlabmgrImpl.cpp -DDEBUG
g++ -g -std=c++0x -c  mempool.cpp	      -DDEBUG
g++ -g -std=c++0x -c  test_mem.cpp       -DDEBUG
g++ test_mem.o mempool.o simpleSlabmgrImpl.o staticPagemgrImpl.o simplePagemgrImpl.o -o test_mem
