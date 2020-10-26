#pragma once
#include "../Core.h"
#include "../Algs/Pseudorand.h"
#include "../Algs/Swapping.h"

///Can be improved

///Forward declaration
template <class T>
class BST;

template <class T>
class BinTreeNode final{
    friend BST<T>;
private:
    BinTreeNode():
        data(T(0)),
        left(nullptr),
        right(nullptr)
    {
    }

    BinTreeNode(const T& data):
        data(data),
        left(nullptr),
        right(nullptr)
    {
    }

    T data;
    BinTreeNode* left;
    BinTreeNode* right;
};

template <class T>
class BinTree{
public:
    enum struct TraversalType{
        InOrder = 0,
        PreOrder,
        PostOrder,
        Amt
    };

    virtual ~BinTree() = 0;
    virtual void AddData(const T& data) = 0;
    virtual void RemoveData(const T& data) = 0;
    virtual void Print(const TraversalType& type) = 0;
    virtual void PrintStructured(const TraversalType& type) = 0;
protected:
    BinTree() = default;
};

template <class T>
BinTree<T>::~BinTree(){}

template <class T>
class BST final: public BinTree<T>{
public:
    BST():
        root(nullptr)
    {
    }

    ~BST(){
        std::function<void(BinTreeNode<T>*&)> PostOrderDel = [&PostOrderDel](BinTreeNode<T>*& node){
            if(node){
                PostOrderDel(node->left);
                PostOrderDel(node->right);
                delete node;
                node = nullptr;
            }
        };
        PostOrderDel(root);
    }

    void AddData(const T& data) override{
        if(!root){
            root = new BinTreeNode<T>(data);
        } else{
            BinTreeNode<T>* temp = root;
            while(true){
                if(data < temp->data){
                    if(temp->left){
                        temp = temp->left;
                    } else{
                        temp->left = new BinTreeNode<T>(data);
                        break;
                    }
                } else if(data > temp->data){
                    if(temp->right){
                        temp = temp->right;
                    } else{
                        temp->right = new BinTreeNode<T>(data);
                        break;
                    }
                } else{
                    if(!temp->left && !temp->right){
                        (PseudorandMinMax(0, 1) ? temp->left : temp->right) = new BinTreeNode<T>(data);
                        break;
                    } else if(!temp->left && temp->right){
                        temp->left = new BinTreeNode<T>(data);
                        break;
                    } else if(temp->left && !temp->right){
                        temp->right = new BinTreeNode<T>(data);
                        break;
                    } else{
                        temp = PseudorandMinMax(0, 1) ? temp->left : temp->right;
                    }
                }
            }
        }
    }

    void RemoveData(const T& data) override{
        BinTreeNode<T>* const& nodeToDel = FindNode(data);
        if(nodeToDel){
            if(!nodeToDel->left && !nodeToDel->right){ //If leaf/external node...
                BinTreeNode<T>* const& parentNode = FindParentNode(nodeToDel);
                if(parentNode){
                    (nodeToDel == parentNode->left ? parentNode->left : parentNode->right) = nullptr;
                    delete nodeToDel;
                } else{
                    delete nodeToDel;
                    root = nullptr;
                }
            } else if(!nodeToDel->left && nodeToDel->right){
                BinTreeNode<T>* const& parentNode = FindParentNode(nodeToDel);
                if(parentNode){
                    (nodeToDel->right->data < parentNode->data ? parentNode->left : parentNode->right) = nodeToDel->right;
                } else{
                    root = nodeToDel->right;
                }
                delete nodeToDel;
            } else if(nodeToDel->left && !nodeToDel->right){
                BinTreeNode<T>* const& parentNode = FindParentNode(nodeToDel);
                if(parentNode){
                    (nodeToDel->left->data < parentNode->data ? parentNode->left : parentNode->right) = nodeToDel->left;
                } else{
                    root = nodeToDel->left;
                }
                delete nodeToDel;
            } else{
                BinTreeNode<T>* currLeft = nodeToDel->left;
                BinTreeNode<T>* currRight = nodeToDel->right;
                while(currLeft->right && currRight->left){ //Can due to properties of BST
                    currLeft = currLeft->right;
                    currRight = currRight->left;
                }
                BinTreeNode<T>* nodeFound = !currLeft->right ? currLeft : currRight;
                if(nodeToDel->data == nodeFound->data){ //Prevents endless/infinite recursion //Not working??
                    if(!currLeft->right){
                        BinTreeNode<T>* parentNode = nodeToDel->left;
                        while(parentNode->right && parentNode->right != nodeFound){
                            parentNode = parentNode->right;
                        }
                        parentNode->right = nodeFound->left;
                    } else{
                        BinTreeNode<T>* parentNode = nodeToDel->right;
                        while(parentNode->left && parentNode->left != nodeFound){
                            parentNode = parentNode->left;
                        }
                        parentNode->left = nodeFound->right;
                    }
                    delete nodeFound;
                    nodeFound = nullptr;
                } else{
                    T nodeData = nodeFound->data;
                    RemoveData(nodeData);
                    nodeToDel->data = nodeData;
                }
            }
        } else{
            std::cout << "'" << data << "' cannot be found in and hence cannot be removed from BST!\n";
        }
    }

    void Print(const BinTree<T>::TraversalType& type) override{ //Wrapper func
        IPrint(root, type);
    }

    void PrintStructured(const BinTree<T>::TraversalType& type) override{ //Wrapper func
        IPrintStructured(root, type, 0, 5);
    }
private:
    BinTreeNode<T>* root;

    BinTreeNode<T>* FindNode(const T& data){
        BinTreeNode<T>* temp = root;
        while(temp && temp->data != data){
            temp = data < temp->data ? temp->left : temp->right;
        }
        return temp;
    }

    BinTreeNode<T>* FindParentNode(const BinTreeNode<T>* const& node){
        BinTreeNode<T>* curr = root;
        BinTreeNode<T>* prev = nullptr;
        while(curr && curr->data != node->data){
            prev = curr;
            curr = node->data < curr->data ? curr->left : curr->right;
        }
        return prev;
    }

    void IPrint(const BinTreeNode<T>* const& node, const BinTree<T>::TraversalType& type){
        if(node){
            switch(type){
                case BinTree<T>::TraversalType::InOrder:
                    IPrint(node->left, type);
                    std::cout << node->data << " ";
                    IPrint(node->right, type);
                    break;
                case BinTree<T>::TraversalType::PreOrder:
                    std::cout << node->data << " ";
                    IPrint(node->left, type);
                    IPrint(node->right, type);
                    break;
                case BinTree<T>::TraversalType::PostOrder:
                    IPrint(node->left, type);
                    IPrint(node->right, type);
                    std::cout << node->data << " ";
                    break;
            }
        }
    }

    void IPrintStructured(const BinTreeNode<T>* const& node, const BinTree<T>::TraversalType& type, const T& amtOfSpaces, const T& spaceWidth){
        if(node){
            IPrintStructured(node->right, type, amtOfSpaces + spaceWidth, spaceWidth);
            for(int i = 0; i < amtOfSpaces; ++i){
                std::cout << " ";
            }
            std::cout << node->data << std::endl;
            IPrintStructured(node->left, type, amtOfSpaces + spaceWidth, spaceWidth);
        }
    }
};

template <class T>
class BinHeap: public BinTree<T>{
public:
    virtual ~BinHeap(){
        if(container){
            delete[] container;
            container = nullptr;
        }
    }

    virtual void AddData(const T& data) override = 0;
    virtual void RemoveData(const T& data) override = 0;

    void Print(const BinTree<T>::TraversalType& type) override{
        IPrint(0, type);
    }

    void PrintStructured(const BinTree<T>::TraversalType& type) override{
        IPrintStructured(0, type, 0, 5);
    }
protected:
    int size;
    int maxSize;
    T* container;

    BinHeap(): BinHeap(999){}
    BinHeap(const int& maxSize):
        size(0),
        maxSize(maxSize),
        container(new T[maxSize])
    {
    }

    int GetParentIndex(const int& index){
        return (index - 1) / 2;
    }

    int GetLeftIndex(const int& index){
        return 2 * index + 1;
    }

    int GetRightIndex(const int& index){
        return 2 * index + 2;
    }

    virtual void Fix(int& index) = 0;
private:
    void IPrint(const int& index, const BinTree<T>::TraversalType& type){
        if(index >= 0 && index < this->size){
            switch(type){
                case BinTree<T>::TraversalType::InOrder:
                    IPrint(GetLeftIndex(index), type);
                    std::cout << this->container[index] << " ";
                    IPrint(GetRightIndex(index), type);
                    break;
                case BinTree<T>::TraversalType::PreOrder:
                    std::cout << this->container[index] << " ";
                    IPrint(GetLeftIndex(index), type);
                    IPrint(GetRightIndex(index), type);
                    break;
                case BinTree<T>::TraversalType::PostOrder:
                    IPrint(GetLeftIndex(index), type);
                    IPrint(GetRightIndex(index), type);
                    std::cout << this->container[index] << " ";
                    break;
            }
        }
    }

    void IPrintStructured(const int& index, const BinTree<T>::TraversalType& type, const T& amtOfSpaces, const T& spaceWidth){
        if(index >= 0 && index < this->size){
            IPrintStructured(GetRightIndex(index), type, amtOfSpaces + spaceWidth, spaceWidth);
            for(int i = 0; i < amtOfSpaces; ++i){
                std::cout << " ";
            }
            std::cout << this->container[index] << std::endl;
            IPrintStructured(GetLeftIndex(index), type, amtOfSpaces + spaceWidth, spaceWidth);
        }
    }
};

template <class T>
class MinHeap final: public BinHeap<T>{
public:
    MinHeap(): BinHeap<T>(){}
    MinHeap(const int& maxSize): BinHeap<T>(maxSize){}
    ~MinHeap() = default;

    void AddData(const T& data) override{
        if(this->size == this->maxSize){
            std::cout << "Failed to add '" << data << "' to full MinHeap!\n";
            return;
        }
        int index = this->size++;
        this->container[index] = data;
        Fix(index);
    }

    void RemoveData(const T& data) override{
        //for(int i = 0; i < this->size; ++i){
        //    if(this->container[index] == data){
        //        SetData(i, std::numeric_limits<T>::min());
        //        return RemoveMin();
        //    }
        //}
        //std::cout << "'" << data << "' cannot be found in and hence cannot be removed from MinHeap!\n";
    }

    //void MinHeap::SetData(const int& index, const T& data){
    //    this->container[index] = data;
    //    Fix(index);
    //}

    //void MinHeap::RemoveMin(){
    //    this->container[0] = this->container[--this->size];
    //    MinHeapify(0);
    //}
private:
    void Fix(int& index) override{
        while(index && this->container[index] < this->container[this->GetParentIndex(index)]){
            SwapArrElements(this->container, index, this->GetParentIndex(index));
            index = this->GetParentIndex(index);
        }
    }
};

template <class T>
class MaxHeap final: public BinHeap<T>{
public:
    MaxHeap(): BinHeap<T>(){}
    MaxHeap(const int& maxSize): BinHeap<T>(maxSize){}
    ~MaxHeap() = default;

    void AddData(const T& data) override{
        if(this->size == this->maxSize){
            std::cout << "Failed to add '" << data << "' to full MaxHeap!\n";
            return;
        }
        int index = this->size++;
        this->container[index] = data;
        Fix(index);
    }

    void RemoveData(const T& data) override{
    }
private:
    void Fix(int& index) override{
        while(index && this->container[index] > this->container[this->GetParentIndex(index)]){
            SwapArrElements(this->container, index, this->GetParentIndex(index));
            index = this->GetParentIndex(index);
        }
    }
};