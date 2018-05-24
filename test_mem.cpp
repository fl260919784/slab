#include "mempool.hpp"
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <set>



void simple_test ()
{
	memMgr::simpleMempool pool;
	if (!pool.initialize(40960, 8, 1.1, 100, 5)) {
		std::cout<<"pool initialize failed"<<std::endl;
		return;
	}

	std::set<char *> mems;
	std::set<char *> ::iterator it;
	long total_size = 0;

	srand((int)time(0));
	int icycle = 0;
	for (;icycle < 10000; ++icycle) {
		int size = rand()%3000;
		char * s = reinterpret_cast<char *>(pool.alloc(size));
		if (!s) {
			printf("out of memory %d\n", icycle);
			continue;
		}
		total_size += size;
		if (!mems.insert(s).second) {
			printf("\tsame addr %p\n", s);
		}
	}

	#if DEBUG
	printf("\n%d==============stat1================%d\n",total_size, mems.size());
	pool.showStat();
	#endif

	printf("=================free=================\n");
	it = mems.begin();
	for (; it != mems.end(); ++it) {
		pool.free(reinterpret_cast<void *>(*it));
	}

	#if DEBUG
	printf("\n==============stat2================\n");
	pool.showStat();
	#endif
}


void static_test ()
{
	memMgr::staticMempool pool;
	if (!pool.initialize(4096*1024 ,4096, 8, 1.1)) {
		std::cout<<"pool initialize failed"<<std::endl;
		return;
	}

	//printf("Debug: initialize end\n");
	std::set<char *> mems;
	std::set<char *> ::iterator it;
	long total_size = 0;

	srand((int)time(0));
	int icycle = 0;
	for (;icycle < 100; ++icycle) {
		int size = rand()%4000;
		char * s = reinterpret_cast<char *>(pool.alloc(size));
		if (!s) {
			printf("out of memory %d\n", icycle);
			continue;
		}
		total_size += size;
		if (!mems.insert(s).second) {
			printf("\tsame addr %p\n", s);
		}
	}

	#if DEBUG
	printf("\n==============stat1================%d\n",mems.size());
	pool.showStat();
	#endif

	printf("=================free=================\n");
	it = mems.begin();
	for (; it != mems.end(); ++it) {
		pool.free(reinterpret_cast<void *>(*it));
	}

	#if DEBUG
	printf("\n==============stat2================\n");
	pool.showStat();
	#endif
}

int main()
{

	//static_test();
	simple_test();
	return 0;
}
