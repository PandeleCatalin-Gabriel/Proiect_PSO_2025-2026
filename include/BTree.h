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
    
};