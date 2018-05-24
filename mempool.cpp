#include "mempool.hpp"
#include "simpleSlabmgrImpl.hpp"
#include "simplePagemgrImpl.hpp"
#include "staticPagemgrImpl.hpp"
#include <malloc.h>



namespace memMgr {
	bool simpleMempool::initialize(unsigned int pageSize, unsigned int baseSize, float factor, 
		 unsigned int high, unsigned int low, int maxSize)
	{
		if (inited) return true;

		do {
			pagemgr = new simplePagemgrImpl(pageSize, high, low, maxSize);
			if (!pagemgr) break;

			simpleSlabmgrImpl * slabimpl = new simpleSlabmgrImpl(pagemgr, baseSize, factor);
			if (!slabimpl) break;

			slabmgr = slabimpl;
			if (!slabimpl->initialize()) break;
			inited = true;

		} while (0);

		if (!inited) {
			printf("Error: initialize failed\n");
			if (pagemgr) delete pagemgr;
			if (slabmgr) delete slabmgr;
			return false;
		}
		
		return true;
	}

	bool simpleMempool::reset()
	{
		if (!inited) return true;

		delete pagemgr;
		delete slabmgr;
		inited = false;

		return true;
	}

	simpleMempool::~simpleMempool()
	{
		reset();
	}

	void * simpleMempool::alloc(int size)
	{
		if (unlikely(!inited)) return NULL;

		return slabmgr->allocItem(size);
	}

	bool simpleMempool::free(void *buf)
	{
		if (unlikely(!inited)) return false;

		return slabmgr->freeItem(reinterpret_cast<char *>(buf));
	}

	#if DEBUG
	void simpleMempool::showStat()
	{
		if (!inited) return;
		pagemgr->showStat();
		slabmgr->showStat();
	}
	#endif


	/**************************************************************************************/


	bool staticMempool::initialize(unsigned int totalSize, unsigned int pageSize, 
				unsigned int baseSize, float factor)
	{
		if (inited) return true;

		do {
			this->pool = reinterpret_cast<char *>(malloc(totalSize));
			if (!this->pool) break;

			pagemgr = new staticPagemgrImpl(this->pool, pageSize, totalSize);
			if (!pagemgr) break;

			simpleSlabmgrImpl * slabimpl = new simpleSlabmgrImpl(pagemgr, baseSize, factor);
			if (!slabimpl) break;

			slabmgr = slabimpl;
			if (!slabimpl->initialize()) break;
			inited = true;

		} while (0);

		if (!inited) {
			printf("Error: initialize failed\n");
			if (pagemgr) delete pagemgr;
			if (slabmgr) delete slabmgr;
			if (this->pool) delete this->pool;
			return false;
		}

		return true;
	}

	bool staticMempool::reset()
	{
		if (!inited) return true;

		delete pagemgr;
		delete slabmgr;
		::free(pool);

		inited = false;
		return true;
	}

	staticMempool::~staticMempool()
	{
		reset();
	}

	void * staticMempool::alloc(int size)
	{
		if (unlikely(!inited)) return NULL;
		return slabmgr->allocItem(size);
	}

	bool   staticMempool::free(void *buf)
	{
		if (unlikely(!inited)) return false;
		return slabmgr->freeItem(reinterpret_cast<char *>(buf));
	}

	#if DEBUG
	void staticMempool::showStat()
	{
		if (!inited) return;
		pagemgr->showStat();
		slabmgr->showStat();
	}
	#endif
};