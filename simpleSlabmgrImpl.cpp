#include "simpleSlabmgrImpl.hpp"
#include "util.hpp"

namespace memMgr {
    simpleSlabmgrImpl::simpleSlabmgrImpl(basePagemgrInterface* pagemgr, 
    	unsigned int baseSize, float factor, int maxSlabNum)
    {
    	this->baseSize 		= baseSize;//非0
        if (this->baseSize < slab::minSize()) {
            //slab因为存在slab_item头，即存放相关slab标记信息，故base_size不能小于slab_item的大小
            this->baseSize = slab::minSize();
        }
    	this->pagemgr   		= pagemgr;
    	this->factor    		= factor;	//不小于1.1
    	maxSlabId 	            = maxSlabNum;
    	inited 		            = false;

    	if (unlikely(this->factor-1.1 <= 0.0)) {
    		log_warning("Warning:@simpleSlabmgrImpl factor too small,use default 1.1\n");
    		this->factor = 1.1;
    	}

    	if (unlikely(this->baseSize == 0)) {
    		log_warning("Warning:@simpleSlabmgrImpl baseSize is zero,use default 2\n");
    		this->baseSize = 2;
    	}

    	if (unlikely(maxSlabId == 0)) {
    		log_warning("Warning:@simpleSlabmgrImpl maxSlabId is zero,use default 1\n");
    		maxSlabId = 1;
    	} else if(unlikely(maxSlabId < 0)) {
            //max_slab_id类型的最大非负数
            maxSlabId = (1<<(sizeof(maxSlabId)*8 - 1))-1;
    	}
    }

    /**
     * 若外部需要
     */
    bool simpleSlabmgrImpl::reset()
    {
        it_slabmgr it = slabManagers.begin();
        for (; it != slabManagers.end(); ++it) {
            
            (*it)->reset();
        }
        return true;
    }

    simpleSlabmgrImpl::~simpleSlabmgrImpl()
    {
    	it_slabmgr it = slabManagers.begin();
        for (; it != slabManagers.end(); ++it) {
            //析构时不需要释放slab中的page,以达到轻量级管理
            //page释放通过pagemgr做整体释放@ 这个是本类的目标功能
            //若希望清空所有 slab的page，则使用reset
            //reset后，当前实例还原至初始状态
            delete(*it);
        }
    }

    bool simpleSlabmgrImpl::initialize()
    {
    	if (inited) return true;

    	int pageSize = pagemgr->getPageSize();
        int start = baseSize;
        int end = pageSize;

        int i = 0;
        for (; i < maxSlabId && start <= end; ++i) {
        	if (start > end / 2) {        //page_size slab
                start = end;
            }

            slab *s = slabFactory::createSlab(pagemgr, i, start, pageSize/start);
            if (unlikely(!s)) {
            	log_error("Error:@simpleSlabmgrImpl out of memory!\n");
            	return false;
            }
            
            log_debug("Debug:simpleSlabmgrImpl slab: id:%d,size:%d,slabNumPerPage:%d,max_slab_id:%d\n", i, start,
                      s->getSlabNumPerPage(), maxSlabId);

            slabManagers.push_back(s);
            slabSizes.push_back(start);

            start = slab::align((int) (start * factor));
        }
        inited = true;
        return true;
    }

    int simpleSlabmgrImpl::getSlabId(int size)
    {
        int slabId = 0;
        it_slabsize it = slabSizes.begin();
        for (;it != slabSizes.end() && *it < size; ++it, ++slabId);

        return slabId;
    }

    simpleSlabmgrImpl::slab *simpleSlabmgrImpl::getSlab(int size)
    {
        int slabId = getSlabId(size);
        if (likely(slabId < (int) slabManagers.size())) {
            return slabManagers[slabId];
        }

        log_debug("Debug:@simpleSlabmgrImpl slab size %d out of range %d\n", 
            size, slabSizes[slabSizes.size()-1]);
        return NULL;
    }

    char * simpleSlabmgrImpl::allocItem(int size)
    {
        if (unlikely(!inited)) return NULL;
    	int itemSize = size + sizeof(slab::slabItem);//将slab_item大小算进去

    	slab *s = getSlab(itemSize);
        if (!s) return NULL;
        
        log_debug("Debug: allocItem@ size:%d,itemSize:%d,slabid：%d,slabsize:%d\n", size, itemSize, s->getSlabId(), s->getSlabSize());
        return s->allocItem();
    }

    /**
     * [simpleSlabmgrImpl::free_item description]
     * @param  item [description]
     * @return      []
     */
    bool simpleSlabmgrImpl::freeItem(char * buf)
    {
        if (!buf) return false;

        char *item = buf - sizeof(slab::slabItem);
        slab::slabItem * si = reinterpret_cast<slab::slabItem*>(item);

        if (unlikely(si->slabId >= slabManagers.size() ||
        	si->slabId < 0)) {
            //此处，可以做未初始化的校验，故开始处无须校验inited
        	log_warning("Warning:freeItem %p unknown slab %d\n", si, si->slabId);
        	return false;
        }
        
        slab *s = slabManagers.at(si->slabId);
        return s->freeItem(item);
    }

    /*==============================slab_factory===================================*/
    simpleSlabmgrImpl::slab * simpleSlabmgrImpl::slabFactory::createSlab(basePagemgrInterface*pagemgr, 
        uint32_t slabId, int slabSize, int num)
    {
    	slab *s = new slab();
        if (unlikely(!s)) {
        	log_error("Error:out of memory!\n");
            return NULL;
        }

        s->setPagemgr(pagemgr);
        s->setSlabId(slabId);
        s->setSlabSize(slabSize);
        s->setSlabNumPerPage(num);
        return s;
    }

    /*==============================slab===================================*/
    bool simpleSlabmgrImpl::slab::reset()
    {
        if (!pagemgr) return true;
        it_setl it = usedPageSet.begin();
        for (; it != usedPageSet.end(); ++it) {
            pagemgr->freePage(reinterpret_cast<char *>(*it));
        }

        usedPageSet.clear();
        usedItemSet.clear();
        freeItemSet.clear();
        return true;
    }

    char * simpleSlabmgrImpl::slab::allocItem()
    {
        int icycle = 0;
        char * result = NULL;
        for (;icycle < 2; ++icycle) {

            //先从当前slab中的free_item中获取空闲的item
            it_setl it_item = freeItemSet.begin();
            if (it_item != freeItemSet.end()) {
                slab::slabItem *item = reinterpret_cast<slab::slabItem *>(*it_item);
                freeItemSet.erase(it_item);
                usedItemSet.insert(*it_item);
                
                pageInfo *page = reinterpret_cast<pageInfo *>(item->pageId);
                page->freeNr --;
                result = item->data;//返回外围时，不包括slab_item信息
                break;
        	}
            
            //无空闲资源，则申请新页
            char * newPage = pagemgr->allocPage();
            if (!newPage) {
                log_error("Error: out of memory\n");
                break;
            }
            usedPageSet.insert(reinterpret_cast<long>(newPage));
            initPage(newPage);
        }

        return result;
    }

    void simpleSlabmgrImpl::slab::clearFreeItem4Page(char *page)
    {
        page += sizeof(pageInfo);
        slab::slabItem *item = reinterpret_cast<slab::slabItem *>(page);
        for (int i = 0; i < slabNumPerPage; ++i) {
            //log_debug("Debug: free item %p belong to page %p\n", item, page-sizeof(pageInfo));
            freeItemSet.erase(reinterpret_cast<long>(item));
            item = reinterpret_cast<slab::slabItem *>((char *) item + slabSize);
        }
    }


    bool simpleSlabmgrImpl::slab::freeItem(char * buf)
    {
        long key = reinterpret_cast<long>(buf);
        it_setl it = usedItemSet.find(key);
        if (likely(it != usedItemSet.end())) {
            slab::slabItem *item = reinterpret_cast<slab::slabItem *>(*it);
            freeItemSet.insert(*it);
            usedItemSet.erase(it);
            
            pageInfo * p = reinterpret_cast<pageInfo*>(item->pageId);
            if (++(p->freeNr) == slabNumPerPage) {
                //整页全部被释放
                log_debug("Debug: reclaim page %p %ld\n", p, item->pageId);
                
                usedPageSet.erase(item->pageId);
                clearFreeItem4Page(reinterpret_cast<char*>(item->pageId));
                pagemgr->freePage(reinterpret_cast<char*>(item->pageId));
            }
            return true;
        }

        //多次释放
        if (freeItemSet.find(key) != freeItemSet.end()) {
            log_warning("Warning: Double free %p\n", buf);
            return false;
        }

        //释放空间不存在
        log_warning("Warning: unknown item %p for free\n", buf);
        return false;
    }

    /**
     * [init_page description]
     * @param page [将page打散成item导入到unused_items中]
     */
    void simpleSlabmgrImpl::slab::initPage(char * page)
    {
        pageInfo * p = reinterpret_cast<pageInfo*>(page);
        p->freeNr = slabNumPerPage;
        long pageId = reinterpret_cast<long>(page);

        page += sizeof(pageInfo);
        slab::slabItem *item = reinterpret_cast<slab::slabItem *>(page);
        for (int i = 0; i < slabNumPerPage; ++i) {
            item->pageId = pageId;
            item->slabId = slabId;
            freeItemSet.insert(reinterpret_cast<long>(item));

            item = reinterpret_cast<slab::slabItem *>((char *) item + slabSize);
        }
    }

    #if DEBUG
    void simpleSlabmgrImpl::showStat()
    {
        it_slabmgr it = slabManagers.begin();
        for (; it != slabManagers.end(); ++it) {
            (*it)->showStat();
        }
    }

    void simpleSlabmgrImpl::slab::showStat()
    {
        log_debug("Debug(slabStat): slabId:%d slabSize:%d slabNumPerPage:%d\n", 
            slabId, slabSize, slabNumPerPage);
        log_debug("\tused_pages:%d,usedItemSet:%d,freeItemSet:%d\n", 
            usedPageSet.size(), usedItemSet.size(), freeItemSet.size());

        int index = 0;
        it_setl it = usedPageSet.begin();
        for (; it != usedPageSet.end(); ++it,++index) {
            pageInfo * page = reinterpret_cast<pageInfo *>(*it);
            log_debug("\tDebug: %d usedPage pageAddr:%p freeNr:%d\n", index, page, page->freeNr);
        }

        it = usedItemSet.begin();
        for (index = 0; it != usedItemSet.end(); ++it,++index) {
            slab::slabItem * item = reinterpret_cast<slab::slabItem *>(*it);
            char * page = reinterpret_cast<char * >(item->pageId);
            log_debug("\tDebug: %d usedItem pageAddr:%p itemAddr:%p slabId:%d\n", index, page, item, item->slabId);
        }


        //it = freeItemSet.begin();
        //for (index = 0; it != freeItemSet.end(); ++it,++index) {
        //    slab::slabItem * item = reinterpret_cast<slab::slabItem *>(*it);
        //    char * page = reinterpret_cast<char * >(item->pageId);
        //    log_debug("\tDebug: %d freeItem pageAddr:%p itemAddr:%p slabId:%d\n", index, page, item, item->slabId);
        //}
    }
    #endif
};