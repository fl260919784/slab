#ifndef __SIMPLE_PAGEMGR_IMPL_H__
#define __SIMPLE_PAGEMGR_IMPL_H__


#include "pagemgrInterface.hpp"
#include <set>
#include <sys/time.h>

namespace memMgr {
    /**
     * 指定内存上限，动态增长的page实现
     * 后续需要实现，空闲页超过一定比例，则释放的功能
     *     一种是超过最大阀值，则主动释放
     *     一种是外围干预释放
     */
    class simplePagemgrImpl :public basePagemgrInterface, 
                        public pagemgrRestable, pagemgrShrinkable {     
        public:
            /**
             * max_size默认为-1表示内存无上限
             */
            simplePagemgrImpl(unsigned int pageSize, unsigned int high, 
                    unsigned int low=0, int maxSize=-1);
            ~simplePagemgrImpl();

            simplePagemgrImpl(simplePagemgrImpl&) = delete;
            simplePagemgrImpl(simplePagemgrImpl&&) = delete;
            simplePagemgrImpl& operator=(const simplePagemgrImpl&) = delete;

        public:
            //override
            char * allocPage() override;
            bool freePage(char *page) override;
            unsigned int getPageSize() override;

            bool reset() override;
            bool shrink() override;

            #if DEBUG
            void showStat() override;
            #endif

        private:
            //空闲页高于watermarkHigh，主动shrink
            unsigned int watermarkHigh;
            //shrink后的空闲页数量
            unsigned int watermarkLow;

            unsigned int pageSize;
            int freePages;
            int usedPages;
            int maxPageNum;

            std::set<long> usedPageSet;
            std::set<long> freePageSet;
            typedef std::set<long>::iterator set_it;
            typedef std::set<long>::reverse_iterator set_rit;
    };
};
#endif