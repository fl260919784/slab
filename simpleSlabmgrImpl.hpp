#ifndef __SIMPLE_SLABMGR_H__
#define __SIMPLE_SLABMGR_H__


#include "pagemgrInterface.hpp"
#include "slabmgrInterface.hpp"
#include <set>
#include <vector>
#include <stdint.h>
#include <sys/time.h>


namespace memMgr {
    /**
     * slab中一但存在空闲页，则释放到pagemgr中
     * simpleSlabmgrImpl类并不做少量的空闲页管理，以达到page的分层管理功能
     *
     *
     * simpleSlabmgrImpl只提供简单的slab管理，simpleSlabmgrImpl析构时，
     *     并不释放page，因为page的释放是pagemgr应有的功能
     *     具体表现为：slab析构时，并未将管理的所有也释放到pagemgr中
     */
    class simpleSlabmgrImpl :public baseSlabmgrInterface, public slabmgrRestable {     
        public:
            /**
             * max_slab_num负数表示最大slab个数只受page_size的影响
             */
            simpleSlabmgrImpl(basePagemgrInterface* page, 
                unsigned int baseSize, float factor, int maxSlabNum=-1);
            ~simpleSlabmgrImpl();

            simpleSlabmgrImpl(simpleSlabmgrImpl&) = delete;
            simpleSlabmgrImpl(simpleSlabmgrImpl&&) = delete;
            simpleSlabmgrImpl& operator=(const simpleSlabmgrImpl&) = delete;

        public:
            bool initialize();
            char *allocItem(int size) override;
            bool freeItem(char *) override;
            bool reset() override;
            #if DEBUG
                void showStat() override;
            #endif

        private:
            class slab {
                public:
                    slab()  = default;
                    ~slab() = default;

                    slab(slab&)                  = delete;
                    slab(slab&&)                 = delete;
                    slab& operator=(const slab&) = delete;

                    char * allocItem();
                    bool freeItem(char * item);
                    bool reset();
                
                public:
                    void setPagemgr(basePagemgrInterface * pagemgr) {
                        this->pagemgr = pagemgr;
                    }

                    void setSlabId(uint32_t id) {
                        this->slabId = id;
                    }
                    uint32_t getSlabId() {
                        return slabId;
                    }
                    void setSlabSize(int size) {
                        slabSize = size;
                    }
                    int getSlabSize() {
                        return slabSize;
                    }
                    void setSlabNumPerPage(int num) {
                        slabNumPerPage =  num;
                    }
                    int getSlabNumPerPage() {
                        return slabNumPerPage;
                    }

                    #if DEBUG
                    void showStat();
                    #endif

                public:
                    static int align(int size) { return (size+alignSize-1) & (~(alignSize-1));}
                    static int minSize() { return align(sizeof(slabItem)+1);}
                    static int slabItemSize() { return sizeof(slabItem);}
                    //static int page_info_size() {return sizeof(page_info);}
                private:
                    static const int alignSize = 8;


                private:
                    void initPage(char * page);
                    void clearFreeItem4Page(char *page);

                public:
                    #pragma pack(1)
                    struct pageInfo {
                        int freeNr;
                        //time_t freeTime; 
                        char data[];
                    };

                    struct slabItem {
                        long pageId;//等同于指针长度
                        int slabId;
                        char data[0];
                    };
                    #pragma pack()

                private:
                    basePagemgrInterface *pagemgr;
                    uint32_t slabId;
                    int slabSize;
                    int slabNumPerPage;

                    std::set<long> usedPageSet;
                    //std::set<long> freePageSet;

                    std::set<long> usedItemSet;
                    std::set<long> freeItemSet;

                    typedef std::set<long>::iterator it_setl;
            };

            class slabFactory {
                public:
                    static slab * createSlab(basePagemgrInterface*pagemgr, uint32_t slabId,
                        int slabSize, int num);
            };
        private:
            slab* getSlab(int size);
            int getSlabId(int size);

        private:
            int maxSlabId;           //最大slab个数
            unsigned int baseSize;    //首个slab的大小
            float factor;              //slab增长因子
            basePagemgrInterface* pagemgr;//上级页式内存管理
            bool inited;


            std::vector<slab *> slabManagers;
            typedef std::vector<slab*>::iterator it_slabmgr;

            std::vector<int> slabSizes;
            typedef std::vector<int>::iterator it_slabsize;

    };
};

#endif