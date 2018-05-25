#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include "pagemgrInterface.hpp"
#include "slabmgrInterface.hpp"


namespace memMgr{
	class simpleMempool {
		public:
			simpleMempool(): inited(false), pagemgr(0), slabmgr(0) {};
			~simpleMempool();

			simpleMempool(simpleMempool&) = delete;
			simpleMempool(simpleMempool&&) = delete;
			simpleMempool& operator=(const simpleMempool&) = delete;

		public:
			bool initialize(unsigned int pageSize, unsigned int baseSize, float factor, 
		 			unsigned int high, unsigned int low=0, int maxSize=-1);
			void * alloc(int size);
			bool  free(void *);

			/**
			 * 重置simpleMempool到构造完成时的状态，释放占用资源
			 */
			bool  reset();

			#if DEBUG
			void showStat();
			#endif

		private:
			bool inited;
			basePagemgrInterface * pagemgr;
			baseSlabmgrInterface * slabmgr;
	};

	class staticMempool {
		public:
			staticMempool(): inited(false), pagemgr(0), slabmgr(0) {};
			~staticMempool();

			staticMempool(staticMempool&) = delete;
			staticMempool(staticMempool&&) = delete;
			staticMempool& operator=(const staticMempool&) = delete;

		public:
			bool initialize(unsigned int totalSize, unsigned int pageSize, 
				unsigned int baseSize, float factor);
			void * alloc(int size);
			bool   free(void *);

			/**
			 * 重置staticMempool到构造完成时的状态，释放占用资源
			 */
			bool  reset();

			#if DEBUG
			void showStat();
			#endif

		private:
			bool inited;
			basePagemgrInterface * pagemgr;
			baseSlabmgrInterface * slabmgr;
			char * pool;
	};

	/**
	 * 后续需求：
	 * 	1、内存池需要有全局受限大小的需求
	 * 	2、中央的空闲page池
	 *  	------可以有个额外的center_pagemgr
	 * 
	 */
};

#endif