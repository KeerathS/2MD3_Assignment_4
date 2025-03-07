#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <cstdlib>
#include <algorithm>
using namespace std;

// We use a string to represent the element since it can be a number, an operatr, or a variable
typedef string Elem;

class LinkedBinaryTree {
protected:
    // Node struct holds each node's data and pointers to its parent and children
    struct Node {
        Elem elt;       // element: number, operator, or variable
        Node* par;      // pointer to the parent node
        Node* left;     // pointer to left child
        Node* right;    // pointer to right child
        Node() : elt(""), par(nullptr), left(nullptr), right(nullptr) {} // default constructor
    };
public:
    // The Position class gives a user-friendly handle to a Node pointer.
    class Position {
    private:
        Node* v; // pointer to the node in the tree
    public:
        Position(Node* _v = nullptr) : v(_v) {}
        Elem& operator*() { return v->elt; } // overloaded * operator to access the element
        Position left() const { return Position(v->left); }  // get left child position
        Position right() const { return Position(v->right); } // get right child position
        Position parent() const { return Position(v->par); }  // get parent position
        bool isRoot() const { return v->par == nullptr; }     // check if this is root
        bool isExternal() const { return v->left == nullptr && v->right == nullptr; } // check if a leaf
        friend class LinkedBinaryTree;
    };
    typedef list<Position> PositionList;
public:
    LinkedBinaryTree();
    // Big Three: copy constructor, assignment operator, and destructor.
    LinkedBinaryTree(const LinkedBinaryTree &other);
    LinkedBinaryTree& operator=(const LinkedBinaryTree &other);
    ~LinkedBinaryTree();

    int size() const;
    bool empty() const;
    Position root() const;
    PositionList positions() const;
    void addRoot();
    void expandExternal(const Position &p);
    Position removeAboveExternal(const Position &p);

    // New methods for expression tree functionality
    void printExpression() const;        // prints the expresion tree in infix form with parentheses
    double evaluateExpression(double a, double b) const; // evaluates the expresion tree given values for a and b
    double getScore() const;               // returns the tree's score
    void setScore(double s);               // sets the tree's score
    bool operator<(const LinkedBinaryTree &other) const; // overload operator for comparing trees by score

    // Friend declaration so that createExpressionTree can access private members.
    friend LinkedBinaryTree createExpressionTree(const string& postfix);

protected:
    void preorder(Node* v, PositionList &pl) const; // recursive helper for traversal
    void printExpression(Node* v) const;            // recursive helper to print the expresion tree
    double evaluateExpression(Node* v, double a, double b) const; // recursive helper to evaluate the tree

private:
    Node* _root;   // pointer to the root node of the tree
    int n;         // number of nodes in the tree
    double score;  // score computed from evaluating the tree (average)

    //CHATGPT
    // Helper functions for deep copy and destruction of nodes.
    Node* clone(Node* v) const;  // recursively clone the tree (CHATGPT was used here to help implement deep copy using recursion)
    void destroy(Node* v);       // recursively delete nodes in the tree
    int countNodes(Node* v) const; // count number of nodes in a subtree
};

//*****************************************************
// Constructor & Basic Methods

LinkedBinaryTree::LinkedBinaryTree() : _root(nullptr), n(0), score(0.0) {}

// Returns the total number of nodes in the tree
int LinkedBinaryTree::size() const { return n; }

// Checks if tree is empty
bool LinkedBinaryTree::empty() const { return size() == 0; }

// Returns a Position representing the root
LinkedBinaryTree::Position LinkedBinaryTree::root() const { return Position(_root); }

// Adds a new root node to an empty tree
void LinkedBinaryTree::addRoot() {
    _root = new Node;
    n = 1;
}

// Expands an external node (leaf) by adding two children nodes
void LinkedBinaryTree::expandExternal(const Position &p) {
    Node* v = p.v;
    v->left = new Node;
    v->left->par = v;
    v->right = new Node;
    v->right->par = v;
    n += 2;
}

// Removes an external node and its parent, replacing them with the sibling node
LinkedBinaryTree::Position LinkedBinaryTree::removeAboveExternal(const Position &p) {
    Node* w = p.v;
    Node* v = w->par;
    Node* sib = (w == v->left ? v->right : v->left);
    if (v == _root) {
        _root = sib;
        sib->par = nullptr;
    } else {
        Node* gpar = v->par;
        if (v == gpar->left)
            gpar->left = sib;
        else
            gpar->right = sib;
        sib->par = gpar;
    }
    delete w;
    delete v;
    n -= 2;
    return Position(sib);
}

// Returns a list of all positions in the tree using preorder traversal
LinkedBinaryTree::PositionList LinkedBinaryTree::positions() const {
    PositionList pl;
    preorder(_root, pl);
    return PositionList(pl);
}

// Helper: Preorder traversal to collect positions
void LinkedBinaryTree::preorder(Node* v, PositionList &pl) const {
    if (v == nullptr) return;
    pl.push_back(Position(v));
    if (v->left != nullptr)
        preorder(v->left, pl);
    if (v->right != nullptr)
        preorder(v->right, pl);
}

//*****************************************************
// New Methods for Expression Trees

// Recursively prints the expresion tree in infix notation with proper parenthesis.
// If the node is a leaf, its value is printed directly. For non-leaf nodes,
// if the node represents the unary operator "abs", it prints it accordingly.
void LinkedBinaryTree::printExpression(Node* v) const {
    if (v == nullptr)
        return;
    // If it's a leaf node, simply print its element.
    if (v->left == nullptr && v->right == nullptr) {
        cout << v->elt;
    } else {
        // For the unary operator "abs"
        if (v->elt == "abs") {
            cout << "abs(";
            printExpression(v->left);
            cout << ")";
        } else {
            // For binary operators, print with parentheses: (left operator right)
            cout << "(";
            printExpression(v->left);
            cout << v->elt; // print the operator
            printExpression(v->right);
            cout << ")";
        }
    }
}

void LinkedBinaryTree::printExpression() const {
    printExpression(_root);
}

// Recursively evaluates the expresion tree using the given values for a and b.
// If the node is a leaf, returns the value of the variable or numeric literal.
// For operator nodes, recursively evalutes subtrees and applies the operator.
// NOTE: For the operator ">", returns 1 if left > right else -1.
double LinkedBinaryTree::evaluateExpression(Node* v, double a, double b) const {
    if (v == nullptr) return 0;
    // If it's a leaf, check if it's a variable or a number.
    if (v->left == nullptr && v->right == nullptr) {
        if (v->elt == "a")
            return a;
        else if (v->elt == "b")
            return b;
        else
            return std::stod(v->elt);  // convert string to double
    } else {
        // For the unary operator "abs"
        if (v->elt == "abs") {
            double val = evaluateExpression(v->left, a, b);
            return (val < 0) ? -val : val;
        }
        // For binary operators, evaluate both left and right subtrees.
        double leftVal = evaluateExpression(v->left, a, b);
        double rightVal = evaluateExpression(v->right, a, b);
        if (v->elt == "+")
            return leftVal + rightVal;
        else if (v->elt == "-")
            return leftVal - rightVal;
        else if (v->elt == "*")
            return leftVal * rightVal;
        else if (v->elt == "/")
            return leftVal / rightVal;
        else if (v->elt == ">")
            return (leftVal > rightVal) ? 1 : -1;
        else
            return 0; // Should not occur (unexpected operator)
    }
}

double LinkedBinaryTree::evaluateExpression(double a, double b) const {
    return evaluateExpression(_root, a, b);
}

// Returns the average score stored in the tree
double LinkedBinaryTree::getScore() const {
    return score;
}

// Sets the tree's score value
void LinkedBinaryTree::setScore(double s) {
    score = s;
}

// Overload the less-than operator to compare trees by score.
// This is useful for sorting trees.
bool LinkedBinaryTree::operator<(const LinkedBinaryTree &other) const {
    return this->score < other.score;
}

//*****************************************************
// Helper Functions for Deep Copy and Destruction

// Recursively clones the subtree rooted at v and returns a pointer to the new clone.
// CHATGPT was used here to design this recursive clone method for deep copying.
LinkedBinaryTree::Node* LinkedBinaryTree::clone(LinkedBinaryTree::Node* v) const {
    if (v == nullptr)
        return nullptr;
    LinkedBinaryTree::Node* newNode = new LinkedBinaryTree::Node;
    newNode->elt = v->elt;
    newNode->left = clone(v->left);
    if(newNode->left != nullptr)
        newNode->left->par = newNode;
    newNode->right = clone(v->right);
    if(newNode->right != nullptr)
        newNode->right->par = newNode;
    return newNode;
}

// Recursively destroys (deletes) the subtree rooted at v to free memory.
void LinkedBinaryTree::destroy(Node* v) {
    if (v != nullptr) {
        destroy(v->left);
        destroy(v->right);
        delete v;
    }
}

// Recursively counts the nodes in the subtree rooted at v.
int LinkedBinaryTree::countNodes(Node* v) const {
    if (v == nullptr)
        return 0;
    return 1 + countNodes(v->left) + countNodes(v->right);
}

//*****************************************************
// Big Three: Copy Constructor, Assignment Operator, Destructor

// Copy constructor that makes a deep copy of the other tree.
// CHATGPT was used here for guidance on recursive deep copying.
LinkedBinaryTree::LinkedBinaryTree(const LinkedBinaryTree &other) : score(other.score) {
    _root = clone(other._root);
    n = countNodes(_root);
}

// Assignment operator that frees existing memory and deep copies the other tree.
LinkedBinaryTree& LinkedBinaryTree::operator=(const LinkedBinaryTree &other) {
    if (this != &other) {
        destroy(_root);
        _root = clone(other._root);
        n = countNodes(_root);
        score = other.score;
    }
    return *this;
}

// Destructor that cleans up all allocated nodes.
LinkedBinaryTree::~LinkedBinaryTree() {
    destroy(_root);
}

//*****************************************************
// Helper Function: Create Expression Tree

//
// This function builds a binary expression tree from a postfix expression string.
// It uses a stack to manage operands and operators. For each token:
//  - If it's an operand, create a single node tree.
//  - If it's an operator, pop one or two trees (depending on whether it is unary or binary)
//    and make them subtrees of a new node containing the operator.
// CHATGPT was used here to quickly devise the stack based algorithm.
LinkedBinaryTree createExpressionTree(const string& postfix) {
    stack<LinkedBinaryTree> s;
    istringstream iss(postfix);
    string token;
    while (iss >> token) {
        // Check if the token is an operator.
        if (token == "abs" || token == "+" || token == "-" || token == "*" || token == "/" || token == ">") {
            if (token == "abs") { // Unary operator
                if (s.empty()) {
                    cerr << "Invalid postfix expression: not enough operands for abs" << endl;
                    exit(1);
                }
                LinkedBinaryTree operandTree = s.top();
                s.pop();
                LinkedBinaryTree T;
                T.addRoot();
                *T.root() = token;
                // Attach the operand tree as the left child.
                T._root->left = operandTree._root;
                if (T._root->left != nullptr)
                    T._root->left->par = T._root;
                // For "abs", the right child is not used.
                T._root->right = nullptr;
                T.n = operandTree.n + 1;
                // Invalidate the operand tree so its destructor won't delete nodes.
                operandTree._root = nullptr;
                operandTree.n = 0;
                s.push(T);
            } else { // Binary operator
                if (s.size() < 2) {
                    cerr << "Invalid postfix expression: not enough operands for " << token << endl;
                    exit(1);
                }
                LinkedBinaryTree rightTree = s.top();
                s.pop();
                LinkedBinaryTree leftTree = s.top();
                s.pop();
                LinkedBinaryTree T;
                T.addRoot();
                *T.root() = token;
                // Attach left and right subtrees.
                T._root->left = leftTree._root;
                if (T._root->left != nullptr)
                    T._root->left->par = T._root;
                T._root->right = rightTree._root;
                if (T._root->right != nullptr)
                    T._root->right->par = T._root;
                T.n = leftTree.n + rightTree.n + 1;
                // Invalidate the old trees so they don't delete nodes in their destructor.
                leftTree._root = nullptr; leftTree.n = 0;
                rightTree._root = nullptr; rightTree.n = 0;
                s.push(T);
            }
        } else {
            // Token is an operand: either a variable ("a" or "b") or a numeric literal.
            LinkedBinaryTree T;
            T.addRoot();
            *T.root() = token;
            T.n = 1;
            s.push(T);
        }
    }
    if (s.size() != 1) {
        cerr << "Invalid postfix expression: remaining trees in stack" << endl;
        exit(1);
    }
    return s.top();
}

//*****************************************************
// Main Function (from the Assignment)

//
// This main function reads postfix expressions from "expressions.txt" and input values from "input.txt".
// It then builds the expression trees, evaluates them with all provided <a, b> pairs,
// computes an average score for each tree, sorts the trees by score, and prints the results.
//
// NOTE: Make sure "expressions.txt" and "input.txt" are in the same working directory as the executable.
int main() {
    // Read postfix expressions into vector
    vector<LinkedBinaryTree> trees;
    ifstream exp_file("expressions.txt");
    string line;
    while (getline(exp_file, line)) {
        if(line.empty()) continue; // Skipping blank lines
        trees.push_back(createExpressionTree(line));
    }
    exp_file.close();

    // Read input data into a 2D vector (each inner vector contains a pair: a and b)
    vector<vector<double> > inputs;
    ifstream input_file("input.txt");
    while (getline(input_file, line)) {
        if(line.empty()) continue; // Skip empty lines
        vector<double> ab_input;
        stringstream ss(line);
        string str;
        while (getline(ss, str, ' ')) {
            if (!str.empty())
                ab_input.push_back(stod(str));
        }
        if(!ab_input.empty())
            inputs.push_back(ab_input);
    }
    input_file.close();

    // Evaluate each expression tree on all provided <a, b> pairs,
    // compute the average, and store it as the tree's score.
    for (auto& t : trees) {
        double sum = 0;
        for (auto& i : inputs) {
            sum += t.evaluateExpression(i[0], i[1]);
        }
        t.setScore(sum / inputs.size());
    }

    // Sort the trees by their score (lowest score first)
    sort(trees.begin(), trees.end());

    // Print out each expression and its computed score.
    for (auto& t : trees) {
        cout << "Exp ";
        t.printExpression();
        cout << " Score " << t.getScore() << endl;
    }

    return 0;
}
