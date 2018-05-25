#ifndef __SLABMGR_INTERFACE_H__
#define __SLABMGR_INTERFACE_H__

namespace memMgr {
    class baseSlabmgrInterface {
        public:
            virtual char *allocItem(int size) = 0;
            virtual bool freeItem(char *) = 0;
            virtual ~baseSlabmgrInterface() = default;

            #if DEBUG
            virtual void showStat() {};
            #endif
    };

    
    //收缩内存，空闲page过多的情况下需要释放一定程度的空闲页
    class slabmgrShrinkable {
        public:
            //watermark min  shrink后，空闲页的持有量
            //watermark low  当空闲页少于low时，shrink不回收空闲页
            //watermark high 当空闲页达到high时，主动触发
            //大于low且小于high时，手动shrink可释放空闲页
            //大于high时，自动回收
            virtual int shrink() = 0;
    };

    //重置slabmgr至初始化完成后的状态
    //即slabmgr构造完成，但尚未分配item时的状态
    class slabmgrRestable {
        public:
            virtual bool reset() = 0;
    };
};
#endif