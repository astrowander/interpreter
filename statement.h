#ifndef STATEMENT_H
#define STATEMENT_H
#include "expressions.h"

//enum Direction  {LEFT, RIGHT};



class Statement
{
private:
    AbstractExpr* currentNode;
    AbstractExpr* root;
    QString codeString;
public:
    Statement(const QString& ss = "")
    {
        root = nullptr;
        currentNode = root;
        codeString = ss;
    }

    ~Statement()
    {
        if (root != nullptr)
            deleteSubTree(root);
    }

    Statement(Statement const& other)
    {
        std::cout<< "Statement copy constructor" << std::endl;
    }

    Statement& operator=(const Statement& other)
    {
        //std::cout<< "Statement assignment" << std::endl;
        root = other.getRoot()->clone();
        QStack<int> path;
        other.getPath(other.getCurrentNode(),path);
        currentNode = getNodeByPath(path);
    }

    void getPath(AbstractExpr* node, QStack<int> &path) const
    {
        if (node->parent==nullptr) return;
        if (node->parent->left == node) path.push(0);
        if (node->parent->right == node) path.push(1);

        for (int i=0; i<node->parent->arguments.size(); ++i)
        {
            if (node->parent->arguments[i]==node) path.push(i+2);
        }

        getPath(node->parent, path);
    }

    AbstractExpr* getNodeByPath(QStack<int>& path) const
    {
        AbstractExpr* ptr = root;
        while (!path.isEmpty())
        {
            int direction = path.pop();

            switch (direction)
            {
            case 0:
                ptr = ptr->left;
                break;
            case 1:
                ptr=ptr->right;
                break;
            default:
                ptr=ptr->arguments[direction-2];
            }
        }
        return ptr;
    }


    void printSubTree(AbstractExpr* node) const
    {
        if (node->left!=nullptr) printSubTree(node->left);
        //std::cout << node->lexem.toStdString() << " ";
        if (node->right!=nullptr) printSubTree(node->right);
    }

    void printTree() const
    {
        std::cout << "Syntax tree: ";
        printSubTree(root);
        std::cout << std::endl;
    }

    void deleteSubTree(AbstractExpr* node)
    {
        AbstractExpr* p = node->parent;       

        if (node->left != nullptr) {
            deleteSubTree(node->left);
        }
        if (node->right != nullptr) {
            deleteSubTree(node->right);
        }

        if (p != nullptr) {
            if (p->left == node) {
                p->left = nullptr;
            }
            if (p->right == node) {
                p->right = nullptr;
            }
        }

        delete node;
        node=nullptr;
    }

    void deleteTree()
    {
        deleteSubTree(root);
        root = nullptr;
        currentNode = nullptr;
    }

    void createNodeAbove(AbstractExpr* newNode) {
        int mode;

        if (currentNode==nullptr) {
            createRightChild(newNode);
            return;
        }

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
        if (currentNode->parent != nullptr)
            currentNode = currentNode->parent;
    }

    AbstractExpr* getRoot() const
    {
        return root;
    }

    AbstractExpr* getCurrentNode() const
    {
        return currentNode;
    }

    const MyVariant& eval() const {
        if (root!=nullptr)
        {
            root->eval();
            return root->value;
        }
        return MyVariant(VOID);
    }

    void setCurrentNodeToRoot()
    {
        currentNode = root;
    }
};


#endif // STATEMENT_H
