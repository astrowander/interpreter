#ifndef STATEMENT_H
#define STATEMENT_H
#include "expressions.h"

class Statement
{
private:
    AbstractExpr* currentNode;
    AbstractExpr* root;
public:
    Statement()
    {
        root = nullptr;
        currentNode = root;
    }

    ~Statement()
    {
        deleteSubTree(root);
    }

    void printSubTree(AbstractExpr* node)
    {
        if (node->left!=nullptr) printSubTree(node->left);
        //std::cout << node->lexem.toStdString() << " ";
        if (node->right!=nullptr) printSubTree(node->right);
    }

    void printTree()
    {
        std::cout << "Syntax tree: ";
        printSubTree(root);
        std::cout << std::endl;
    }

    void deleteSubTree(AbstractExpr* node)
    {
        AbstractExpr* p = node->parent;
        if (p != nullptr) {
            if (p->left == node) {
                p->left = nullptr;
            }
            if (p->right == node) {
                p->right = nullptr;
            }
        }

        if (node->left != nullptr) {
            deleteSubTree(node->left);
        }
        if (node->right != nullptr) {
            deleteSubTree(node->right);
        }
        delete node;
    }

    void createNodeAbove(AbstractExpr* newNode) {
        int mode;

        if (currentNode->parent==nullptr) {
            mode=0;
        }
        else if (currentNode->parent->left == currentNode) {
            mode=1;
        }
        else if (currentNode->parent->right == currentNode) {
            mode=2;
        }
        else {
            mode=-1;
        }
        //AbstractExpr* oldParent = currentNode->parent;
        newNode->left = currentNode;
        newNode->parent = currentNode->parent;
        currentNode->parent = newNode;

        switch (mode) {
        case 0:
            root = currentNode->parent;
            break;
        case 1:
            currentNode->parent->parent->left = currentNode->parent;
            break;
        case 2:
            currentNode->parent->parent->right = currentNode->parent;
            break;
        case -1:
            std::cout << "Pointers error has occured" << std::endl;
        }

    }

    void createLeftChild(AbstractExpr* newNode)
    {
       currentNode->left = newNode;
    }

    void createRightChild(AbstractExpr* newNode)
    {
       if (currentNode==nullptr) {
           newNode->parent=nullptr;
           newNode->left=nullptr;
           newNode->right=nullptr;
           currentNode=newNode;
           root=currentNode;
           return;
       }
       newNode->parent = currentNode;
       currentNode->right = newNode;
       goRight();
    }

    void goLeft()
    {
        currentNode = currentNode->left;
    }

    void goRight()
    {
        currentNode = currentNode->right;
    }

    void goUp()
    {
        currentNode = currentNode->parent;
    }

    AbstractExpr* getRoot()
    {
        return root;
    }

    MyVariant eval() {
        return root->eval();
    }
};


#endif // STATEMENT_H
