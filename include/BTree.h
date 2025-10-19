#pragma once
#include<string>
#include<vector>
#include<memory>
#include<functional>

template<typename T, typename V>
class BTreeNode
{
public:
    std::vector<T> keys;
    std::vector<V> values;
    std::vector<std::shared_ptr<BTreeNode<T,V>>> children;
    bool isLeaf;
    int minDegree;

    BTreeNode(int degree, bool leaf): minDegree(degree),isLeaf(leaf){} 

    V search(const T& key);
    void insertNonFull(const T& key,const V& value);
    void splitChild(int i,std::shared_ptr<BTreeNode<T,V>> child);
};

template<typename T,typename V>
class BTree
{
private:
    std::shared_ptr<BTreeNode<T,V>> root;
    int minDegree;
public:
    BTree(int degree = 3): minDegree(degree),root(nullptr){}

    V search (const T& key);
    void insert(const T& key, const V& value);
    void remove(const T& key);
};

template <typename T, typename V>
inline V BTreeNode<T, V>::search(const T &key)
{
    int i=0;
    while(i<keys.size() && *key > *keys[i])
    {
        i++;
    }
    if (i < keys.size() && *keys[i] == *key)
        return values[i];
    
    if (isLeaf)
        return nullptr;
    
    return children[i]->search(key);
}

template <typename T, typename V>
inline void BTreeNode<T, V>::insertNonFull(const T &key, const V &value)
{
    int i = keys.size() - 1;
    
    if (isLeaf) {
        keys.push_back(T());
        values.push_back(V());
        
        while (i >= 0 && *keys[i] > *key) {
            keys[i + 1] = keys[i];
            values[i + 1] = values[i];
            i--;
        }
        
        keys[i + 1] = key;
        values[i + 1] = value;
    } else {
        while (i >= 0 && *keys[i] > *key)
            i--;
        
        if (children[i + 1]->keys.size() == 2 * minDegree - 1) {
            splitChild(i + 1, children[i + 1]);
            if (*keys[i + 1] < *key)
                i++;
        }
        children[i + 1]->insertNonFull(key, value);
    }
}

template <typename T, typename V>
inline void BTreeNode<T, V>::splitChild(int i, std::shared_ptr<BTreeNode<T, V>> child)
{
    auto newNode = std::make_shared<BTreeNode<T,V>>(child->minDegree, child->isLeaf);
    
    for (int j = 0; j < minDegree - 1; j++) {
        newNode->keys.push_back(child->keys[j + minDegree]);
        newNode->values.push_back(child->values[j + minDegree]);
    }
    
    if (!child->isLeaf) {
        for (int j = 0; j < minDegree; j++)
            newNode->children.push_back(child->children[j + minDegree]);
    }
    
    keys.insert(keys.begin() + i, child->keys[minDegree - 1]);
    values.insert(values.begin() + i, child->values[minDegree - 1]);
    children.insert(children.begin() + i + 1, newNode);
    
    child->keys.resize(minDegree - 1);
    child->values.resize(minDegree - 1);
    if (!child->isLeaf)
        child->children.resize(minDegree);
}

template<typename T, typename V>
V BTree<T,V>::search(const T& key) 
{
    return root == nullptr ? nullptr : root->search(key);
}

template<typename T, typename V>
void BTree<T,V>::insert(const T& key, const V& value) 
{
    if (root == nullptr) {
        root = std::make_shared<BTreeNode<T,V>>(minDegree, true);
        root->keys.push_back(key);
        root->values.push_back(value);
    } else {
        if (root->keys.size() == 2 * minDegree - 1) {
            auto newRoot = std::make_shared<BTreeNode<T,V>>(minDegree, false);
            newRoot->children.push_back(root);
            newRoot->splitChild(0, root);
            
            int i = 0;
            if (*newRoot->keys[0] < *key)
                i++;
            newRoot->children[i]->insertNonFull(key, value);
            
            root = newRoot;
        } else {
            root->insertNonFull(key, value);
        }
    }
}

template<typename T, typename V>
void BTree<T,V>::remove(const T& key) 
{
    //de facut la final
}