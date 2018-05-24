#include "staticPagemgrImpl.hpp"
#include <stdint.h>

namespace memMgr {
    staticPagemgrImpl::staticPagemgrImpl(char *pool, 
            unsigned int pageSize, unsigned int totalSize)
    {
        this->pool = pool;        
        this->pageSize = pageSize;
        totalPages = totalSize/pageSize;
        freePages = totalPages;
        currentPage = 0;
        
        bitmap.reserve(totalPages);
        unsigned int index = 0;
        for (; index < totalPages; ++index) {
            this->bitmap.push_back(false);
        }
        printf("Debug: totalPages:%d,pageSize:%d,totalSize:%d\n", 
            totalPages, pageSize, totalSize);
    }

    bool staticPagemgrImpl::reset()
    {
        this->pool = NULL;        
        this->pageSize = 0;
        totalPages = 0;
        freePages = 0;
        currentPage = 0;
        bitmap.clear();

        return true;
    }

    unsigned int staticPagemgrImpl::getPageSize()
    {
        return pageSize;
    }

    char * staticPagemgrImpl::index2Page(int index)
    {
        uint64_t offset = static_cast<uint64_t> (index);
        offset *= pageSize;
        return pool + offset;
    }

    char *staticPagemgrImpl::allocPage()
    {
        if (!freePages) return NULL;

        /*
         * find a free page index from bitmap
         */
        for (;; ++currentPage) {
            if (currentPage == totalPages) {
                currentPage = 0;
            }
            if (!bitmap[currentPage]) {
                break;
            }
        }
        
        --freePages;
        bitmap[currentPage] = true;
        return index2Page(currentPage);
    }

    int staticPagemgrImpl::page2Index(const char *page)
    {
        uint64_t offset = page - pool;
        return offset / static_cast<uint64_t> (pageSize);
    }

    bool staticPagemgrImpl::freePage(char *page)
    {
        if (unlikely(!page)) {
            printf("Error: free page is NUll");
            return false;
        }
        int index = page2Index(page);
        if (unlikely(index < 0 || index >= totalPages)) {
            printf("Error: free page out of range(bitmap)");
            return false; 
        }

        bitmap[index] = false;
        ++freePages;
        return true;
    }

    #if DEBUG
    void staticPagemgrImpl::showStat()
    {
        printf("DEBUG:(staticPagemgrImpl) pageSize:%d, totalPages:%d freePages:%d, currentPage:%d\n", 
            pageSize, totalPages, freePages, currentPage);

        int index = 0;
        it_bitmap it = bitmap.begin();
        for (; it != bitmap.end(); ++it, ++index) {
            printf("\tDEBUG: page:%d %s\n", index, *it?"used":"free");
        }
    }
    #endif
};