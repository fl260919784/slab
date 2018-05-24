#ifndef __PAGEMGR_INTERFACE_H__
#define __PAGEMGR_INTERFACE_H__
#include <stdio.h>

#define likely(x)                   __builtin_expect(!!(x), 1)
#define unlikely(x)                 __builtin_expect(!!(x), 0)

namespace memMgr {
    class basePagemgrInterface {
        public:
            virtual char * allocPage() = 0;
            virtual bool freePage(char *page) = 0;
            virtual unsigned int getPageSize() = 0;
            virtual ~basePagemgrInterface() = default;

            #if DEBUG
             virtual void showStat() = 0;
            #endif
    };

    //收缩内存，空闲page过多的情况下需要释放一定程度的空闲页
    class pagemgrShrinkable {
        public:
            //watermark low  当空闲页少于low时，shrink不回收空闲页
            //watermark high 当空闲页达到high时，主动触发
            //大于low且小于high时，手动shrink可释放空闲页
            //大于high时，自动回收
            virtual bool shrink() = 0;
    };

    //释放所有内存页
    class pagemgrRestable {
        public:
            virtual bool reset() = 0;
    };


    //针对shrink，后续可以将当前mempage空闲页shrink到中央mempage中
    

    //如何实现全局内存大小受限的pagemgr呢？？？？
};
#endif