#ifndef __STATIC_PAGEMGR_IMPL_H__
#define __STATIC_PAGEMGR_IMPL_H__

#include "pagemgrInterface.hpp"
#include <vector>


namespace memMgr {
    /**
     * 传入连续内存空间，进行分页管理
     * 注意：因为内存时入参传入的，故staticPagemgrImpl析构时，不对其进行释放
     *     具体操作由外围内存池提供
     */
    class staticPagemgrImpl :public basePagemgrInterface, public pagemgrRestable {
        public:
            //ctor dtor etc..
            staticPagemgrImpl(char *pool, unsigned int pageSize, unsigned int totalSize);
            ~staticPagemgrImpl() = default;

            staticPagemgrImpl(staticPagemgrImpl&) = delete;
            staticPagemgrImpl(staticPagemgrImpl&&) = delete;
            staticPagemgrImpl& operator=(const staticPagemgrImpl&) = delete;

        public:
            //override
            char * allocPage() override;
            bool freePage(char *page) override;
            unsigned int getPageSize() override;
            bool reset() override;

            #if DEBUG
            void showStat() override;
            #endif

        private:
            char * index2Page(int index);
            int page2Index(const char * page);

        private:
            unsigned int pageSize;
            unsigned int totalPages;
            unsigned int freePages;
            unsigned int currentPage;
            char *pool;

            std::vector<bool> bitmap;
            typedef std::vector<bool>::iterator it_bitmap;
    };
};
#endif