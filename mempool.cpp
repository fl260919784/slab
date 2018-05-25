#include "mempool.hpp"
#include "simpleSlabmgrImpl.hpp"
#include "simplePagemgrImpl.hpp"
#include "staticPagemgrImpl.hpp"
#include "util.hpp"

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
			log_warning("Warning: initialize failed\n");
			if (pagemgr) delete pagemgr;
			if (slabmgr) delete slabmgr;

			slabmgr = NULL;
			pagemgr = NULL;
		}
		
		return inited;
	}

	bool simpleMempool::reset()
	{
		if (!inited) return true;

		dynamic_cast<slabmgrRestable*>(slabmgr)->reset();
		dynamic_cast<pagemgrRestable*>(pagemgr)->reset();
		
		return true;
	}

	simpleMempool::~simpleMempool()
	{
		if (!inited) return;
		
		delete slabmgr;
		delete pagemgr;//由pagemgr的析构来触发页的释放
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
			log_warning("Warning: initialize failed\n");
			if (pagemgr) delete pagemgr;
			if (slabmgr) delete slabmgr;
			if (this->pool) ::free(this->pool);

			slabmgr = NULL;
			pagemgr = NULL;
			this->pool = NULL;

		}

		return inited;
	}

	bool staticMempool::reset()
	{
		if (!inited) return true;

		dynamic_cast<slabmgrRestable*>(slabmgr)->reset();
		dynamic_cast<pagemgrRestable*>(pagemgr)->reset();

		return true;
	}

	staticMempool::~staticMempool()
	{
		::free(this->pool);
		delete slabmgr;
		delete pagemgr;
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