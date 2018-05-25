#include "mempool.hpp"
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <set>
#include <unistd.h>



void simple_test ()
{
	memMgr::simpleMempool pool;
	if (!pool.initialize(40960, 8, 1.1, 1024, 100)) {
		std::cout<<"pool initialize failed"<<std::endl;
		return;
	}

	std::set<char *> mems;
	std::set<char *> ::iterator it;
	long total_size = 0;
	int runtimes = 0;
	for (; runtimes < 3; ++runtimes) {
		mems.clear();
		printf("\n\n===================runtimes:%d===============\n",runtimes);
		srand((int)time(0));
		int icycle = 0;
		for (icycle = 0 ;icycle < 10000; ++icycle) {
			int size = (rand()%4000 + rand()%4000 + 1)%3000;
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

		printf("=========alloc totail size %d, item size %d========\n", total_size, mems.size());
		#if DEBUG
		pool.showStat();
		#endif

		printf("=========free %d items========\n", mems.size()/2);
		int size = mems.size();
		
		for (icycle = 0;!mems.empty() && icycle<size/2;++icycle) {
			it = mems.begin();
			pool.free(reinterpret_cast<void *>(*it));
			mems.erase(it);
		}

		printf("=========item size %d========\n", mems.size());	
		#if DEBUG
		pool.showStat();
		#endif


		for (icycle = 0 ;icycle < 10000; ++icycle) {
			int size = (rand()%4000 + rand()%4000 + 1)%3000;
			char * s = reinterpret_cast<char *>(pool.alloc(size));
			if (!s) {
				printf("out of memory %d\n", icycle);
				continue;
			}
			if (!mems.insert(s).second) {
				printf("\tsame addr %p\n", s);
			}
		}
		printf("=========continue alloc end:item size %d========\n", mems.size());

		
		printf("=========free al items:%d========\n", mems.size());
		
		for (;!mems.empty();) {
			it = mems.begin();
			pool.free(reinterpret_cast<void *>(*it));
			mems.erase(it);
		}

		printf("=========item size %d========\n", mems.size());	

		#if DEBUG
		pool.showStat();
		#endif
		sleep(2);
	}
	sleep(2);
}


void static_test ()
{
	memMgr::staticMempool pool;
	if (!pool.initialize(4096*20480 ,4096, 8, 1.1)) {
		std::cout<<"pool initialize failed"<<std::endl;
		return;
	}

	std::set<char *> mems;
	std::set<char *> ::iterator it;
	long total_size = 0;
	int runtimes = 0;
	for (; runtimes < 3; ++runtimes) {
		mems.clear();
		printf("\n\n===================runtimes:%d===============\n",runtimes);
		srand((int)time(0));
		int icycle = 0;
		for (icycle = 0 ;icycle < 10000; ++icycle) {
			int size = (rand()%4000 + rand()%4000 + 1)%3000;
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

		printf("=========alloc totail size %d, item size %d========\n", total_size, mems.size());
		#if DEBUG
		pool.showStat();
		#endif

		printf("=========free %d items========\n", mems.size()/2);
		int size = mems.size();
		
		for (icycle = 0;!mems.empty() && icycle<size/2;++icycle) {
			it = mems.begin();
			pool.free(reinterpret_cast<void *>(*it));
			mems.erase(it);
		}

		printf("=========item size %d========\n", mems.size());	
		#if DEBUG
		pool.showStat();
		#endif


		for (icycle = 0 ;icycle < 10000; ++icycle) {
			int size = (rand()%4000 + rand()%4000 + 1)%3000;
			char * s = reinterpret_cast<char *>(pool.alloc(size));
			if (!s) {
				printf("out of memory %d\n", icycle);
				continue;
			}
			if (!mems.insert(s).second) {
				printf("\tsame addr %p\n", s);
			}
		}
		printf("=========continue alloc end:item size %d========\n", mems.size());

		
		printf("=========free al items:%d========\n", mems.size());
		
		for (;!mems.empty();) {
			it = mems.begin();
			pool.free(reinterpret_cast<void *>(*it));
			mems.erase(it);
		}

		printf("=========item size %d========\n", mems.size());	

		#if DEBUG
		pool.showStat();
		#endif
		sleep(2);
	}
	sleep(2);
}

int main()
{

	static_test();
	//simple_test();
	return 0;
}
