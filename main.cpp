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

// Change the base element type to string for our expression tree
typedef string Elem;

class LinkedBinaryTree {
protected:
    struct Node {
        Elem elt;       // element (number, operator, or variable)
        Node* par;      // parent pointer
        Node* left;     // left child
        Node* right;    // right child
        Node() : elt(""), par(NULL), left(NULL), right(NULL) {}
    };
public:
    class Position {
    private:
        Node* v; // pointer to the node
    public:
        Position(Node* _v = NULL) : v(_v) {}
        Elem& operator*() { return v->elt; }
        Position left() const { return Position(v->left); }
        Position right() const { return Position(v->right); }
        Position parent() const { return Position(v->par); }
        bool isRoot() const { return v->par == NULL; }
        bool isExternal() const { return v->left == NULL && v->right == NULL; }
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
    void printExpression() const;
    double evaluateExpression(double a, double b) const;
    double getScore() const;
    void setScore(double s);
    bool operator<(const LinkedBinaryTree &other) const;

    // Declare createExpressionTree as friend so it can access private members.
    friend LinkedBinaryTree createExpressionTree(const string& postfix);

protected:
    void preorder(Node* v, PositionList &pl) const;
    void printExpression(Node* v) const;
    double evaluateExpression(Node* v, double a, double b) const;

private:
    Node* _root;
    int n;
    double score; // Member variable to store the tree's score

    // Helper functions for deep copy and destruction.
    Node* clone(Node* v) const;
    void destroy(Node* v);
    int countNodes(Node* v) const;
};

///////////////////////////
// Constructor & Basic Methods
///////////////////////////
LinkedBinaryTree::LinkedBinaryTree() : _root(NULL), n(0), score(0.0) {}

int LinkedBinaryTree::size() const { return n; }

bool LinkedBinaryTree::empty() const { return size() == 0; }

LinkedBinaryTree::Position LinkedBinaryTree::root() const { return Position(_root); }

void LinkedBinaryTree::addRoot() {
    _root = new Node;
    n = 1;
}

void LinkedBinaryTree::expandExternal(const Position &p) {
    Node* v = p.v;
    v->left = new Node;
    v->left->par = v;
    v->right = new Node;
    v->right->par = v;
    n += 2;
}

LinkedBinaryTree::Position LinkedBinaryTree::removeAboveExternal(const Position &p) {
    Node* w = p.v;
    Node* v = w->par;
    Node* sib = (w == v->left ? v->right : v->left);
    if (v == _root) {
        _root = sib;
        sib->par = NULL;
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

LinkedBinaryTree::PositionList LinkedBinaryTree::positions() const {
    PositionList pl;
    preorder(_root, pl);
    return PositionList(pl);
}

void LinkedBinaryTree::preorder(Node* v, PositionList &pl) const {
    if (v == NULL) return;
    pl.push_back(Position(v));
    if (v->left != NULL)
        preorder(v->left, pl);
    if (v->right != NULL)
        preorder(v->right, pl);
}

///////////////////////////
// New Methods for Expression Trees
///////////////////////////

// Recursive helper to print an expression tree with parentheses.
void LinkedBinaryTree::printExpression(Node* v) const {
    if (v == NULL)
        return;
    // If the node is a leaf, simply print its element.
    if (v->left == NULL && v->right == NULL) {
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
            cout << v->elt;
            printExpression(v->right);
            cout << ")";
        }
    }
}

void LinkedBinaryTree::printExpression() const {
    printExpression(_root);
}

// Recursive helper to evaluate an expression tree given values for a and b.
double LinkedBinaryTree::evaluateExpression(Node* v, double a, double b) const {
    if (v == NULL) return 0;
    // Leaf node: either a variable or a numeric literal.
    if (v->left == NULL && v->right == NULL) {
        if (v->elt == "a")
            return a;
        else if (v->elt == "b")
            return b;
        else
            return std::stod(v->elt);
    } else {
        // For the unary operator "abs"
        if (v->elt == "abs") {
            double val = evaluateExpression(v->left, a, b);
            return (val < 0) ? -val : val;
        }
        // For binary operators, evaluate both subtrees.
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
            return 0; // Should not occur.
    }
}

double LinkedBinaryTree::evaluateExpression(double a, double b) const {
    return evaluateExpression(_root, a, b);
}

double LinkedBinaryTree::getScore() const {
    return score;
}

void LinkedBinaryTree::setScore(double s) {
    score = s;
}

bool LinkedBinaryTree::operator<(const LinkedBinaryTree &other) const {
    return this->score < other.score;
}

///////////////////////////
// Helper Functions for Deep Copy and Destruction
///////////////////////////

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


// Recursively destroys the subtree rooted at v.
void LinkedBinaryTree::destroy(Node* v) {
    if (v != nullptr) {
        destroy(v->left);
        destroy(v->right);
        delete v;
    }
}

// Recursively counts nodes in the subtree.
int LinkedBinaryTree::countNodes(Node* v) const {
    if (v == nullptr)
        return 0;
    return 1 + countNodes(v->left) + countNodes(v->right);
}

///////////////////////////
// Big Three: Copy Constructor, Assignment Operator, Destructor
///////////////////////////
LinkedBinaryTree::LinkedBinaryTree(const LinkedBinaryTree &other) : score(other.score) {
    _root = clone(other._root);
    n = countNodes(_root);
}

LinkedBinaryTree& LinkedBinaryTree::operator=(const LinkedBinaryTree &other) {
    if (this != &other) {
        destroy(_root);
        _root = clone(other._root);
        n = countNodes(_root);
        score = other.score;
    }
    return *this;
}

LinkedBinaryTree::~LinkedBinaryTree() {
    destroy(_root);
}

///////////////////////////
// Helper Function: Create Expression Tree
///////////////////////////
//
// This function builds a binary expression tree from a postfix expression string.
// It uses a stack of trees and attaches subtrees based on whether the operator is unary (abs)
// or binary (+, -, *, /, >).
//
LinkedBinaryTree createExpressionTree(const string& postfix) {
    stack<LinkedBinaryTree> s;
    istringstream iss(postfix);
    string token;
    while (iss >> token) {
        // Check if token is an operator.
        if (token == "abs" || token == "+" || token == "-" || token == "*" || token == "/" || token == ">") {
            if (token == "abs") { // Unary operator.
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
                if (T._root->left != NULL)
                    T._root->left->par = T._root;
                // For "abs", we do not use the right child.
                T._root->right = NULL;
                T.n = operandTree.n + 1;
                // Invalidate the operand tree so its destructor does not free nodes.
                operandTree._root = NULL;
                operandTree.n = 0;
                s.push(T);
            } else { // Binary operator.
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
                if (T._root->left != NULL)
                    T._root->left->par = T._root;
                T._root->right = rightTree._root;
                if (T._root->right != NULL)
                    T._root->right->par = T._root;
                T.n = leftTree.n + rightTree.n + 1;
                // Invalidate the old trees so they don't free nodes.
                leftTree._root = NULL; leftTree.n = 0;
                rightTree._root = NULL; rightTree.n = 0;
                s.push(T);
            }
        } else {
            // Token is an operand: either a variable ("a" or "b") or a number.
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

///////////////////////////
// Main Function (from the Assignment)
///////////////////////////
//
// This main function reads postfix expressions from "expressions.txt" and input values from "input.txt".
// It then builds the expression trees, evaluates them with all the provided <a, b> pairs,
// computes an average score for each tree, sorts the trees by score, and prints the results.
//
int main() {
    // Read postfix expressions into vector
    vector<LinkedBinaryTree> trees;
    ifstream exp_file("expressions.txt");
    string line;
    while (getline(exp_file, line)) {
        if(line.empty()) continue; // Skip empty lines.
        trees.push_back(createExpressionTree(line));
    }
    exp_file.close();

    // Read input data into 2D vector
    vector<vector<double> > inputs;
    ifstream input_file("input.txt");
    while (getline(input_file, line)) {
        if(line.empty()) continue;
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

    // Evaluate expressions on input data
    for (auto& t : trees) {
        double sum = 0;
        for (auto& i : inputs) {
            sum += t.evaluateExpression(i[0], i[1]);
        }
        t.setScore(sum / inputs.size());
    }

    // Sort trees by their score (lowest score first)
    sort(trees.begin(), trees.end());

    // Print out each expression and its score.
    for (auto& t : trees) {
        cout << "Exp ";
        t.printExpression();
        cout << " Score " << t.getScore() << endl;
    }

    return 0;
}
