#include "simplePagemgrImpl.hpp"
#include <malloc.h>

namespace memMgr {
    simplePagemgrImpl::simplePagemgrImpl(unsigned int pageSize, unsigned int high, 
                    unsigned int low, int maxSize)
    {
        this->pageSize = pageSize;

        if (maxSize >= 0)
            maxPageNum = maxSize/pageSize;
        else
            maxPageNum = -1;//传入负数时，表示内存不受限
        
        watermarkHigh = high;
        watermarkLow  = low;  
    }

    unsigned int simplePagemgrImpl::getPageSize()
    {
        return pageSize;
    }

    char *simplePagemgrImpl::allocPage()
    {
        char * result = NULL;

        set_it it = freePageSet.begin();
        if (it != freePageSet.end()) {
            result = reinterpret_cast<char *>(*it);
            freePageSet.erase(it);
            return result;
        }

        //若设置了max_page_num（非负），则校验上线，否则不校验
        if (unlikely(maxPageNum >= 0 && usedPages >= maxPageNum)) {
            printf("Debug:@simplePagemgrImpl page number is limited\n");
            return NULL;
        }

        result = static_cast<char *>(malloc(pageSize));
        if (unlikely(!result)) {
            printf("Error:@simplePagemgrImpl out of memory\n");
            return NULL;
        }

        usedPageSet.insert(reinterpret_cast<long>(result));
        usedPages++;
        return result;
    }

    bool simplePagemgrImpl::freePage(char *page)
    {
        long key = reinterpret_cast<long>(page);
        if (unlikely(!usedPageSet.erase(key))) {
            printf("Error:@simplePagemgrImpl page %p unknown\n", page);
            return false;
        }
        usedPages--;

        if (unlikely(!freePageSet.insert(key).second)) {
            printf("Error:@simplePagemgrImpl page %p Double free\n", page);
            return false;
        }
        freePages++;

        printf("Debug: freePages:%d watermarkHigh:%d\n", freePages, watermarkHigh);
        if (freePages >= watermarkHigh) {
            shrink();
        }

        return true;
    }

    /**
     * 释放所有管理页
     */
    bool simplePagemgrImpl::reset()
    {
        printf("Debug:@simplePagemgrImpl rest\n");
        set_it it = usedPageSet.begin();
        for (;it != usedPageSet.end(); ++it) {
            ::free(reinterpret_cast<char *>(*it));
        }

        it = freePageSet.begin();
        for (;it != freePageSet.end(); ++it) {
            ::free(reinterpret_cast<char *>(*it));
        }

        usedPageSet.clear();
        freePageSet.clear();
        freePages = 0;
        usedPages = 0;
        return 0;
    }

    bool simplePagemgrImpl::shrink()
    {
        printf("Debug: begin shrink: low:%d freePages:%d:%d high:%d\n", 
            watermarkLow, freePageSet.size(), freePages, watermarkHigh);
        /**
         * 反向释放，保留低地址空间内存
         * 乱序释放会导致内存碎片的概率高
         */
        for (;!freePageSet.empty() && freePages>watermarkLow;--freePages) {
            set_rit it = freePageSet.rbegin();
            free(reinterpret_cast<char *>(*it));
            freePageSet.erase(*it);
        }

        printf("Debug: end shrink: low:%d freePages:%d high:%d\n", 
            watermarkLow, freePageSet.size(), watermarkHigh);
        return true;
    }


    simplePagemgrImpl::~simplePagemgrImpl()
    {
        reset();
    }

    #if DEBUG
    void simplePagemgrImpl::showStat()
    {
        printf("DEBUG(simplePagemgrImpl):pageSize:%d, maxPageNum:%d usedPages:%d, freePages:%d\n", 
            pageSize, maxPageNum, usedPages, freePages);

        char * p = NULL;
        set_it it = usedPageSet.begin();
        for (; it != usedPageSet.end(); ++it) {
            p = reinterpret_cast<char *>(*it);
            printf("\tDEBUG: used page_addr: %p\n", p);
        }

        it = freePageSet.begin();
        for (; it != freePageSet.end(); ++it) {
            p = reinterpret_cast<char *>(*it);
            printf("\tDEBUG: free page_addr: %p\n", p);
        }
    }
    #endif
};