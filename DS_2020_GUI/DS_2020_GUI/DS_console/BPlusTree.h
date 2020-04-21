﻿#pragma once
#include "pch.h"
#include "BPlusTreeUtils.h"
/* @by ChrisJaunes
 * 以下是B+树节点信息的注释
 * self: 自身文件指针
 * parent: 父节点文件指针
 * type: 节点类型包括内部节点(BPLUSTREE_INTERNAL)和叶子节点(BPLUSTREE_LEAF)
 * ch_cnt: 孩子的个数,根据B+树定义:内部节点有ch_cnt-1个key、cn_cnt个孩子指针; 叶子节点有ch_cnt个key、cn_cnt个数据域指针
 * prev: 同级上一个节点的文件指针,链表
 * next: 同级下一个节点的文件指针,链表
 * keys[]：key数组
 * chds[]: 孩子数组的文件指针
 * isleaf(): 判断是否是叶子节点
 * binary_search_by_key()： 用于处理key上的二分
 * fromFileBlock(): 用于从文件块中读取数据并且借助读取的数据生成一个BPlusTreeNode节点
 * toFileBlock(): 用于向文件块中写入数据并且借助自身的节点信息写入数据
 *
 * 以下是B+树信息的注释
 * index_fd: 节点存储文件的文件指针
 * index_fdpath: 节点存储文件的文件路径
 * index_fdsz: 节点存储文件的文件大小
 * data_fd: 数据存储文件的文件指针
 * data_fdpath: 数据存储文件的文件路径
 * data_fdsz: 数据存储文件的文件大小
 * root: 树根的文件指针
 * level: 树高
 * leaf_head: 叶子节点链的文件指针
 * 手动内存管理
 * BPT_BLOCK_SIZE：BPlusTreeNode块大小
 * caches: 内存管理
 * used[]: 内存管理
 * cache_refer: 获取一块内存空间
 * cache_refer: 释放一块内存空间
 * 以下是B+树文件信息
 *  起始块 64 byte
 * | 偏移 | 大小 |内容
 * |---   |---  |---
 * |0	  | 8	 | 魔数, -DS,V1-； 检验信息

 *
 * 节点块 包括内部节点和叶子节点
 * | 偏移 | 大小 |内容
 * |---   |---  |---
 * |0	  | 8	| sizeof(size_t) 自身文件偏移,用于检验,对应self
 * |8	  | 8	| sizeof(size_t) 父节点文件偏移,用于检验,对应parent
 * |16	  | 4	| sizeof(int) 类型, 内部节点或者叶子节点, 对应type
 * |20	  | 4	| sizeof(int) 该节点孩子数, 对应ch_cnt
 * |24	  | 8	| sizeof(size_t) 同级链上一个节点的文件偏移, 对应prev
 * |32	  | 8	| sizoef(size_t) 同级链下一个节点的文件偏移, 对应next
 * |40	  | ？	| sizoef(key_t) * BPT_MAX_ORDER, 对应keys数组
 * |？    | ？  | sizoef(chd_t) * BPT_MAX_ORDER, 对应chds数组
 *
 * 数据块
 * | 偏移 | 大小 |内容
 * |---   |---  |---
 * |0	  | 8	| sizeof(size_t) 自身文件偏移,用于检验
 * |8	  | 8	| sizeof(size_t) 下一个数据块节点, 本质上是一个链表
 * |16	  | ？	| 数据内容, 二进制文件
 */

template<typename KEY_T = key_t>
class BPlusTree {
public:
    explicit BPlusTree(const wchar_t* filepath, int exist = BPLUSTREE_FILE_NON_EXIST, int _node_block_size = BPT_NODE_BLOCK_SIZE, int _data_block_size = BPT_DATA_BLOCK_SIZE);
    BPlusTree(const BPlusTree& a);
    void operator=(const BPlusTree& a);
    ~BPlusTree();
    int insert(KEY_T key, const void* value, const size_t value_sz);
    int search(KEY_T key, void*& value, size_t& value_sz);
//private:
    //节点类
    struct BPlusTreeNode {
        OFF_T self;
        OFF_T parent;
        unsigned int type;
        unsigned int ch_cnt;
        OFF_T prev;
        OFF_T next;
        KEY_T keys[BPT_MAX_ORDER];
        OFF_T chds[BPT_MAX_ORDER];
        BPlusTreeNode(const BPlusTreeNode&);
        void operator=(const BPlusTreeNode& a);
        int binary_search_by_key(const KEY_T& key);
    };
    //B+树基本信息
    void* BPT_caches;
    unsigned int node_block_size;
    unsigned int data_block_size;
    OFF_T root;
    unsigned int level;
    unsigned int number;
    OFF_T leaf_head;
    OFF_T node_block_head;
    OFF_T data_block_head;
    void treeFromFileBlock();
    void treeToFileBlock();
    FILE* fd;
    //节点块缓存池
    void* node_caches;
    size_t node_used_cnt;
    std::unordered_map<OFF_T, void*> node_map;
    std::unordered_map<void*, size_t> node_used;
    std::map<size_t, void*> node_unused;
    void node_lock();
    void node_unlock();
    void* node_refer();
    void node_defer(void* node);
    OFF_T node_empty_block();
    void nodeFromFileBlock(BPlusTreeNode* node, OFF_T file_off);
    void nodeToFileBlock(BPlusTreeNode* node, OFF_T file_off);
    BPlusTreeNode* node_new(OFF_T _self, OFF_T _parent,int _type);
    BPlusTreeNode* node_fetch(OFF_T chdset);
    void node_flush_parent(OFF_T node, BPlusTreeNode* parent);
    //数据块缓存池
    void* data_caches;
    size_t data_used_cnt;
    std::unordered_map<OFF_T, void*> data_map;
    std::unordered_map<void*, size_t> data_used;
    std::map<size_t, void*> data_unused;
    void data_lock();
    void data_unlock();
    void* data_refer();
    void data_defer(void*);
    OFF_T data_empty_block();
    void dataFromFileBlock(OFF_T file_off);
    void dataToFileBlock(OFF_T file_of);
    void* data_fetch(OFF_T chdset);
    void valueFromFileBlock(OFF_T file_off, void*& value, size_t& value_sz);
    OFF_T valueToFileBlock(OFF_T file_off, const void* value, const size_t value_sz);

    //临时使用缓存池
    unsigned int temp_block_size;
    void* temp_caches;
    int temp_used[BPT_CACHE_TEMP_NUM];
    void* swap_refer();
    void swap_defer(void*);

    KEY_T node_split(BPlusTreeNode* node, const int pos, const KEY_T key, const OFF_T value_off, BPlusTreeNode*& lch, BPlusTreeNode*& rch);
    void node_insert(BPlusTreeNode* node, const int pos, const KEY_T key, const OFF_T value_off);
};

template<typename KEY_T>
inline BPlusTree<KEY_T>::BPlusTree(const wchar_t* filepath, int exist, int _node_block_size, int _data_block_size)
{
    size_t sz = wcslen(filepath) + 1;
    wchar_t* fdpath = new wchar_t[sz];
    memset(fdpath, 0, sz);
    wcscat(fdpath, filepath);

    BPT_caches = malloc(BPT_ROOT_SIZE);

    if (exist) {
        assert((fd = _wfopen(fdpath, L"rb+")) != nullptr);
        treeFromFileBlock();
    }
    else {
        assert((fd = _wfopen(fdpath, L"wb+")) != nullptr);
        node_block_size = _node_block_size;
        data_block_size = _data_block_size;
        assert(sizeof(BPlusTreeNode) <= node_block_size);
        root = INVALID_OFFSET;
        level = 0;
        number = 0;
        leaf_head = INVALID_OFFSET;
        node_block_head = INVALID_OFFSET;
        data_block_head = INVALID_OFFSET;
        treeToFileBlock();
    }

    sz = (size_t) node_block_size * BPT_CACHE_NUM;
    node_caches = malloc(sz);
    node_used_cnt = 0;
    for (int i = 0; i < BPT_CACHE_NUM; i++) {
        node_unused[++node_used_cnt] = (char*)node_caches + (size_t)node_block_size * i;
    }
    sz = (size_t)data_block_size * BPT_CACHE_NUM;
    data_caches = malloc(sz);
    data_used_cnt = 0;
    for (int i = 0; i < BPT_CACHE_NUM; i++) {
        data_unused[++data_used_cnt] = (char*)data_caches + (size_t)data_block_size * i;
    }
    temp_block_size = max(node_block_size, data_block_size) * 2;
    temp_caches = malloc((size_t)temp_block_size * BPT_CACHE_TEMP_NUM);
    memset(temp_used, 0, sizeof(temp_used));
    
    delete[] fdpath;
}

template<typename KEY_T>
inline BPlusTree<KEY_T>::BPlusTree(const BPlusTree& a)
{
    assert(0);
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::operator=(const BPlusTree& a)
{
    assert(0);
}

template<typename KEY_T>
inline BPlusTree<KEY_T>::~BPlusTree()
{
    for (auto it : node_map) nodeToFileBlock((BPlusTreeNode*)it.second, ((BPlusTreeNode*)it.second)->self);
    //for (auto it : data_map) nodeToFileBlock(it.second);
    treeToFileBlock();
    free(BPT_caches);
    free(node_caches);
    free(data_caches);
    free(temp_caches);
    fclose(fd);
}

template<typename KEY_T>
inline int BPlusTree<KEY_T>::insert(KEY_T key, const void* value, const size_t value_sz)
{
    BPlusTreeNode* node = node_fetch(root);
    if (node == nullptr) {
        node = node_new(node_empty_block(), INVALID_OFFSET, BPLUSTREE_LEAF);
        node->ch_cnt = 1;
        node->keys[0] = key;
        node->chds[0] = valueToFileBlock(INVALID_OFFSET, value, value_sz);

        root = node->self;
        level = 1;
        ++number;
        leaf_head = node->self;
        node_defer(node);
        return BPLUSTRE_SUCCESS;
    }
    int pos;
    while (node->type == BPLUSTREE_INTERNAL) {
        pos = node->binary_search_by_key(key);
        OFF_T next_off = node->chds[pos];
        node_defer(node);
        node = node_fetch(next_off);
    }

    pos = node->binary_search_by_key(key);
    if (pos < node->ch_cnt && node->keys[pos] == key) {
        valueToFileBlock(node->chds[pos], value, value_sz);
        node_defer(node);
        return BPLUSTRE_SUCCESS_REPLACE;
    }
    OFF_T value_off = valueToFileBlock(INVALID_OFFSET, value, value_sz);
    BPlusTreeNode* lch, * rch;
    while (true) {
        if (node->ch_cnt < BPT_MAX_ORDER) {
            node_insert(node, pos, key, value_off);
            node_defer(node);
            return BPLUSTRE_SUCCESS;;
        }
        key = node_split(node, pos, key, value_off, lch, rch);
        if (lch->parent == INVALID_OFFSET && rch->parent == INVALID_OFFSET) {
            node = node_new(node_empty_block(), INVALID_OFFSET, BPLUSTREE_INTERNAL);
            node->ch_cnt = 2;
            node->keys[0] = key;
            node->chds[0] = lch->self;
            node->chds[1] = rch->self;
            lch->parent = node->self;
            rch->parent = node->self;
            ++number;
            root = node->self;
            ++level;
            node_defer(lch);
            node_defer(rch);
            node_defer(node);
            return BPLUSTRE_SUCCESS;
        }
        value_off = lch->self;
        node = node_fetch(lch->parent);
        pos = node->binary_search_by_key(key);
        node_defer(lch);
        node_defer(rch);
    }
    assert(0);
    return BPLUSTRE_FAILED;
}

template<typename KEY_T>
inline int BPlusTree<KEY_T>::search(KEY_T key, void*& value, size_t& value_sz)
{
    BPlusTreeNode* node = node_fetch(root);
    if (node == NULL) {
        return BPLUSTRE_FAILED;
    }
    int pos;
    while (node->type == BPLUSTREE_INTERNAL) {
        pos = node->binary_search_by_key(key);
        OFF_T next_off = node->chds[pos];
        node_defer(node);
        node = node_fetch(next_off);
    }
    pos = node->binary_search_by_key(key);
    if (pos < node->ch_cnt && node->keys[pos] == key) {
        valueFromFileBlock(node->chds[pos], value, value_sz);
        node_defer(node);
        return BPLUSTRE_SUCCESS;
    }
    node_defer(node);
    return BPLUSTRE_FAILED;
}

template<typename KEY_T>
inline BPlusTree<KEY_T>::BPlusTreeNode::BPlusTreeNode(const BPlusTreeNode&)
{
    assert(0);
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::BPlusTreeNode::operator=(const BPlusTreeNode& a)
{
    assert(0);
}

template<typename KEY_T>
inline int BPlusTree<KEY_T>::BPlusTreeNode::binary_search_by_key(const KEY_T& key)
{
    int len = type == BPLUSTREE_LEAF ? ch_cnt : ch_cnt - 1;
    int pos = std::lower_bound(keys, keys + len, key) - keys;
    return pos;
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::treeFromFileBlock()
{
    BPlusTreeUtils::db_read(fd, 0, BPT_caches, BPT_ROOT_SIZE, 1);

    char magic_number[8];
    assert(sscanf((char*)BPT_caches, "%s", &magic_number) == 1);
    assert(strcmp(magic_number, MAGIC_NUMBER) == 0);
    size_t off = 8;
    memmove(&node_block_size, (char*)BPT_caches + off, sizeof(node_block_size)); off += sizeof(node_block_size);
    memmove(&data_block_size, (char*)BPT_caches + off, sizeof(data_block_size)); off += sizeof(data_block_size);
    memmove(&root,            (char*)BPT_caches + off, sizeof(root)           ); off += sizeof(root);
    memmove(&level,           (char*)BPT_caches + off, sizeof(level)          ); off += sizeof(level);
    memmove(&number,          (char*)BPT_caches + off, sizeof(number)         ); off += sizeof(number);
    memmove(&leaf_head,       (char*)BPT_caches + off, sizeof(leaf_head)      ); off += sizeof(leaf_head);
    memmove(&node_block_head, (char*)BPT_caches + off, sizeof(node_block_head)); off += sizeof(node_block_head);
    memmove(&data_block_head, (char*)BPT_caches + off, sizeof(data_block_head)); off += sizeof(data_block_head);
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::treeToFileBlock()
{
    sprintf((char*)BPT_caches, "%s", MAGIC_NUMBER);
    size_t off = 8;
    memmove((char*)BPT_caches + off, &node_block_size, sizeof(node_block_size)); off += sizeof(node_block_size);
    memmove((char*)BPT_caches + off, &data_block_size, sizeof(data_block_size)); off += sizeof(data_block_size);
    memmove((char*)BPT_caches + off, &root,            sizeof(root)           ); off += sizeof(root);
    memmove((char*)BPT_caches + off, &level,           sizeof(level)          ); off += sizeof(level);
    memmove((char*)BPT_caches + off, &number,          sizeof(number)         ); off += sizeof(number);
    memmove((char*)BPT_caches + off, &leaf_head,       sizeof(leaf_head)      ); off += sizeof(leaf_head);
    memmove((char*)BPT_caches + off, &node_block_head, sizeof(node_block_head)); off += sizeof(node_block_head);
    memmove((char*)BPT_caches + off, &data_block_head, sizeof(data_block_head)); off += sizeof(data_block_head);
    BPlusTreeUtils::db_write(fd, 0, BPT_caches, BPT_ROOT_SIZE, 1);
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::node_lock()
{
    //实现多线程的安全问题
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::node_unlock()
{
    //实现多线程的安全问题
}

template<typename KEY_T>
inline void* BPlusTree<KEY_T>::node_refer()
{
    assert(!node_unused.empty());
    BPlusTreeNode* node = (BPlusTreeNode * )node_unused.begin()->second;
    if (node_used.count(node)) {
        node_map.erase(node->self);
        nodeToFileBlock(node, node->self);
    }
    node_unused.erase(node_unused.begin());
    node_used[node] = ++node_used_cnt;
    return node;
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::node_defer(void* node)
{
    assert(node != nullptr);
    assert(node_used.count(node));
    node_unused[node_used[node]] = node;
}

template<typename KEY_T>
inline OFF_T BPlusTree<KEY_T>::node_empty_block()
{
    //总所周知, 只有插入和查询的B+树，节点只会插在文件尾部
    void* buf = swap_refer();
    assert(node_block_head == INVALID_OFFSET);
    OFF_T res = BPlusTreeUtils::db_EOF(fd);
    memset(buf, 'A', node_block_size);
    BPlusTreeUtils::db_write(fd, res, buf, node_block_size, 1);
    swap_defer(buf);
    return res;
}

//为了防止结构体对齐原则不一致
template<typename KEY_T>
inline void BPlusTree<KEY_T>::nodeFromFileBlock(BPlusTreeNode* node, OFF_T file_off)
{
    void* buf = swap_refer();
    BPlusTreeUtils::db_read(fd, file_off, buf, node_block_size, 1);
    size_t off = 0;
    memmove(&node->self, (char*)buf + off, sizeof(node->self)); off += sizeof(node->self);
    memmove(&node->parent, (char*)buf + off, sizeof(node->parent)); off += sizeof(node->parent);
    memmove(&node->type, (char*)buf + off, sizeof(node->type)); off += sizeof(node->type);
    memmove(&node->ch_cnt, (char*)buf + off, sizeof(node->ch_cnt)); off += sizeof(node->ch_cnt);
    memmove(&node->prev, (char*)buf + off, sizeof(node->prev)); off += sizeof(node->prev);
    memmove(&node->next, (char*)buf + off, sizeof(node->next)); off += sizeof(node->next);
    memmove(&node->keys, (char*)buf + off, sizeof(node->keys)); off += sizeof(node->keys);
    memmove(&node->chds, (char*)buf + off, sizeof(node->chds)); off += sizeof(node->chds);
    swap_defer(buf);
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::nodeToFileBlock(BPlusTreeNode* node, OFF_T file_off)
{
    assert(node->self == file_off);
    void* buf = swap_refer();
    size_t off = 0;
    memmove((char*)buf + off, &node->self, sizeof(node->self)); off += sizeof(node->self);
    memmove((char*)buf + off, &node->parent, sizeof(node->parent)); off += sizeof(node->parent);
    memmove((char*)buf + off, &node->type, sizeof(node->type)); off += sizeof(node->type);
    memmove((char*)buf + off, &node->ch_cnt, sizeof(node->ch_cnt)); off += sizeof(node->ch_cnt);
    memmove((char*)buf + off, &node->prev, sizeof(node->prev)); off += sizeof(node->prev);
    memmove((char*)buf + off, &node->next, sizeof(node->next)); off += sizeof(node->next);
    memmove((char*)buf + off, &node->keys, sizeof(node->keys)); off += sizeof(node->keys);
    memmove((char*)buf + off, &node->chds, sizeof(node->chds)); off += sizeof(node->chds);
    BPlusTreeUtils::db_write(fd, file_off, buf, node_block_size, 1);
    swap_defer(buf);
}

template<typename KEY_T>
inline typename BPlusTree<KEY_T>::BPlusTreeNode* BPlusTree<KEY_T>::node_new(OFF_T _self, OFF_T _parent, int _type)
{
    assert(_self != INVALID_OFFSET);
    BPlusTreeNode* node = (BPlusTreeNode*)node_refer();

    node->self = _self;
    node->parent = _parent;
    node->type = _type;
    node->ch_cnt = 0;
    node->prev = INVALID_OFFSET;
    node->next = INVALID_OFFSET;
    node_map[node->self] = node;
    return node;
}

template<typename KEY_T>
inline typename BPlusTree<KEY_T>::BPlusTreeNode* BPlusTree<KEY_T>::node_fetch(OFF_T chdset)
{
    if (chdset == INVALID_OFFSET) {
        return nullptr;
    }
    if (node_map.count(chdset)) {
        BPlusTreeNode* node = (BPlusTreeNode * )node_map[chdset];
        size_t _id = node_used[node];
        assert(node_unused.count(_id));
        //if (unused.count(_id)) 
        node_unused.erase(_id);
        node_used[node] = ++node_used_cnt;
        return node;
    }
    BPlusTreeNode* node = (BPlusTreeNode *)node_refer();
    nodeFromFileBlock(node, chdset);
    node_map[node->self] = node;
    return node;
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::node_flush_parent(OFF_T node, BPlusTreeNode* parent)
{
    BPlusTreeNode* child = node_fetch(node);
    assert(child != NULL);
    child->parent = parent->self;
    node_defer(child);
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::data_lock()
{
    //待实现
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::data_unlock()
{
    //待实现
}

template<typename KEY_T>
inline void* BPlusTree<KEY_T>::data_refer()
{
    //待实现
    assert(!node_used.count(data_caches));
    node_used[data_caches] = 1;
    return data_caches;
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::data_defer(void*)
{
    //待实现
    node_used.erase(data_caches);
}

template<typename KEY_T>
inline OFF_T BPlusTree<KEY_T>::data_empty_block()
{
    //待实现
    OFF_T res;
    void* buf = swap_refer();
    if (data_block_head == INVALID_OFFSET) {
        res = BPlusTreeUtils::db_EOF(fd);
        BPlusTreeUtils::db_write(fd, res, buf, data_block_size, 1);
    }
    else {
        res = data_block_head;
        BPlusTreeUtils::db_read(fd, data_block_head, buf, data_block_size, 1);
        memmove(&data_block_head, (char*)buf + sizeof(data_block_head), sizeof(data_block_head));
    }
    swap_defer(buf);
    return res;
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::dataFromFileBlock(OFF_T file_off)
{
    //待实现
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::dataToFileBlock(OFF_T file_of)
{
    //待实现
}

template<typename KEY_T>
inline void* BPlusTree<KEY_T>::data_fetch(OFF_T chdset)
{
    //待实现
    return NULL;
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::valueFromFileBlock(OFF_T file_off, void*& value, size_t& value_sz)
{
    assert(value == NULL);

    void* buf = swap_refer();
    void* value_tmp;
    value = NULL;
    value_sz = 0;
    size_t sz = data_block_size - sizeof(OFF_T) * 2;
    while (file_off != INVALID_OFFSET) {
        BPlusTreeUtils::db_read(fd, file_off, buf, data_block_size, 1);
        memmove(&file_off, (char*)buf + sizeof(OFF_T), sizeof(OFF_T));

        value_tmp = value;
        value = malloc(value_sz + sz);
        memmove(value, (char*)buf + sizeof(OFF_T) * 2, sz);
        memmove((char*)value + value_sz, value_tmp, value_sz);
        value_sz += sz;
        free(value_tmp);
    }
    swap_defer(buf);
}

template<typename KEY_T>
inline OFF_T BPlusTree<KEY_T>::valueToFileBlock(OFF_T file_off, const void* value, const size_t value_sz)
{
    void* buf = swap_refer();
    while (file_off != INVALID_OFFSET) {
        BPlusTreeUtils::db_read(fd, file_off, buf, data_block_size, 1);
        memmove(&file_off, (char*)buf + sizeof(OFF_T), sizeof(OFF_T));
        memmove((char*)buf + sizeof(OFF_T), &data_block_head, sizeof(OFF_T));
        memmove(&data_block_head, buf, sizeof(OFF_T));
    }

    size_t value_sz_tmp = 0;
    size_t sz = sizeof(OFF_T);
    assert(value_sz % (data_block_size - sz * 2) == 0);

    while (value_sz_tmp < value_sz) {
        memmove((char*)buf + sz, &file_off, sz);
        file_off = data_empty_block();
        memmove(buf, &file_off, sz);
        memmove((char*)buf + sz * 2, (char*)value + value_sz_tmp, data_block_size - sz * 2);
        BPlusTreeUtils::db_write(fd, file_off, buf, data_block_size, 1);
        value_sz_tmp += data_block_size - sz * 2;
    }
    swap_defer(buf);
    return file_off;
}

template<typename KEY_T>
inline void* BPlusTree<KEY_T>::swap_refer()
{
    for (int i = 0; i < BPT_CACHE_TEMP_NUM; i++) {
        if (!temp_used[i]) {
            temp_used[i] = 1;
            return (char*)temp_caches + temp_block_size * i;
        }
    }
    assert(0);
    return nullptr;
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::swap_defer(void* node)
{
    if (node == nullptr) return;
    int i = ((char*)node - temp_caches) / temp_block_size;
    temp_used[i] = 0;
}

template<typename KEY_T>
inline KEY_T BPlusTree<KEY_T>::node_split(typename BPlusTreeNode* node, const int pos, const KEY_T key, const OFF_T value_off, BPlusTreeNode*& lch, BPlusTreeNode*& rch)
{
    assert(node->ch_cnt == BPT_MAX_ORDER);
    BPlusTreeNode* sibling = node_new(node_empty_block(), node->parent, node->type);

    int split = (BPT_MAX_ORDER + 1) / 2;
    sibling->ch_cnt = split;
    node->ch_cnt = BPT_MAX_ORDER - split + 1;

    if ((sibling->prev = node->prev) != INVALID_OFFSET) {
        BPlusTreeNode* prev = node_fetch(sibling->prev);
        prev->next = sibling->self;
        node_defer(prev);
    }
    else {
        leaf_head = sibling->self;
    }
    node->prev = sibling->self;
    sibling->next = node->self;
    void* buf = swap_refer();
    size_t sz = sizeof(KEY_T), off = 0;
    memmove((char*)buf + off, &(node->keys[0])  , pos * sz); off += pos * sz;
    memmove((char*)buf + off, &key              , sz      ); off += sz;
    memmove((char*)buf + off, &(node->keys[pos]), (BPT_MAX_ORDER - pos) * sz);
    off = 0;
    memmove(&(sibling->keys[0]), (char*) buf + off, sibling->ch_cnt * sz); off += sibling->ch_cnt * sz;
    memmove(&(node->keys[0]),    (char*) buf + off, node->ch_cnt * sz);

    sz = sizeof(OFF_T); off = 0;
    memmove((char*)buf + off, &(node->chds[0]), pos * sz); off += pos * sz;
    memmove((char*)buf + off, &value_off      , sz      ); off += sz;
    memmove((char*)buf + off, &(node->chds[pos]), (BPT_MAX_ORDER - pos) * sz);
    off = 0;
    memmove(&(sibling->chds[0]), (char*) buf + off, sibling->ch_cnt * sz); off += sibling->ch_cnt * sz;
    memmove(&(node->chds[0]),    (char*) buf + off, node->ch_cnt * sz);
    swap_defer(buf);
    if (sibling->type == BPLUSTREE_INTERNAL) {
        for (int i = 0; i < sibling->ch_cnt; i++)
            node_flush_parent(sibling->chds[i], sibling);
    }

    KEY_T res = sibling->keys[split - 1];
    lch = sibling;
    rch = node;
    
    return res;
}

template<typename KEY_T>
inline void BPlusTree<KEY_T>::node_insert(BPlusTreeNode* node, const int pos, const KEY_T key, const OFF_T value_off)
{
    memmove(&(node->keys[pos + 1]), &(node->keys[pos]), (node->ch_cnt - pos) * sizeof(KEY_T));
    memmove(&(node->chds[pos + 1]), &(node->chds[pos]), (node->ch_cnt - pos) * sizeof(OFF_T));
    node->keys[pos] = key;
    node->chds[pos] = value_off;
    node->ch_cnt++;
    if (node->type == BPLUSTREE_INTERNAL) node_flush_parent(value_off, node);
}
