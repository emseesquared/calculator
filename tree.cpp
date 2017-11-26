#include <stdexcept>

#include "tree.h"
#include "exceptions.h"

void TreeNode::addChildRoutine(TreeNode** ptrToChild, TreeNode* node) {
  delete *ptrToChild;

  *ptrToChild = node;
  if (node)
    node->setParent(this);
}

TreeNode* TreeNode::popChildRoutine(TreeNode** ptrToChild) {
  TreeNode* oldChild = *ptrToChild;

  if (oldChild) {
    oldChild->setParent(nullptr);
    *ptrToChild = nullptr;
  }

  return oldChild;
}

short Operator::binaryPriority() const {
  switch (type_) {
  case '+':
  case '-': return 1;
  case '*':
  case '/': return 2;
  default: throw std::runtime_error("Unknown binary priority");
  }
}

short Operator::unaryPriority() const {
  switch (type_) {
  case '+':
  case '-': return 3;
  default: throw std::runtime_error("Unknown unary priority");
  }
}

bool Operator::canBeUnary() const {
  switch (type_) {
  case '+':
  case '-': return true;
  default: return false;
  }
}

double Operator::operator()(const double arg) const {
  switch (type_) {
  case '+': return arg;
  case '-': return -arg;
  default: throw std::runtime_error("Unknown unary operator");
  }
}

double Operator::operator()(const double lhs, const double rhs) const {
  switch (type_) {
  case '+': return lhs + rhs;
  case '-': return lhs - rhs;
  case '*': return lhs * rhs;
  case '/': return lhs / rhs;
  default: throw std::runtime_error("Unknown binary operator");
  }
}

short RootNode::getPriority() const {
  throw std::runtime_error("Root has no priority");
}

void RootNode::addChild(TreeNode* node) {
  addChildRoutine(&firstChild_, node);
}

TreeNode* RootNode::popChild() {
  return popChildRoutine(&firstChild_);
}

RootNode::~RootNode() {
  delete firstChild_;
}

double RootNode::evaluate() const {
  // Empty expression evaluates to zero
  if (!filled())
    throw std::runtime_error("Evaluating an empty tree");

  return firstChild_->evaluate();
}

short Leaf::getPriority() const {
  throw std::runtime_error("Leafs have no priority");
}

void Leaf::addChild(TreeNode*) {
  throw std::runtime_error("Leafs have no children");
}

TreeNode* Leaf::popChild() {
  throw std::runtime_error("Leafs have no children");
}

UnaryNode::~UnaryNode() {
  delete child_;
}

void UnaryNode::addChild(TreeNode* node) {
  addChildRoutine(&child_, node);
}

TreeNode* UnaryNode::popChild() {
  return popChildRoutine(&child_);
}

double UnaryNode::evaluate() const {
  if (!filled())
    throw std::runtime_error("Evaluating unary node without an operand");

  return operator_(child_->evaluate());
}

BinaryNode::~BinaryNode() {
  delete leftChild_;
  delete rightChild_;
}

void BinaryNode::addChild(TreeNode* node) {
  if (!leftChild_)
    addChildRoutine(&leftChild_, node);
  else
    addChildRoutine(&rightChild_, node);
}

TreeNode* BinaryNode::popChild() {
  if (!rightChild_)
    popChildRoutine(&leftChild_);
  else
    popChildRoutine(&rightChild_);
}

double BinaryNode::evaluate() const {
  if (!filled())
    throw std::runtime_error("Evaluating binary node without operand(s)");

  return operator_(leftChild_->evaluate(),
		   rightChild_->evaluate());
}

EvaluationTree::EvaluationTree(): root_(new RootNode()), insertionPoint_(root_) { }

EvaluationTree::~EvaluationTree() {
  delete root_;
}

EvaluationTree::EvaluationTree(EvaluationTree&& rhs): root_(rhs.root_), insertionPoint_(rhs.insertionPoint_) {
  rhs.root_ = nullptr;
  rhs.insertionPoint_ = nullptr;
}


bool EvaluationTree::isReady() const {
  return rootReached() || insertionPoint_->filled();
}

void EvaluationTree::insertOperand(const OperandType& arg) {
  if (insertionPoint_->filled())
    throw Exceptions::UnexpectedOperand();
  
  insertionPoint_->addChild(new Leaf(arg));
}

void EvaluationTree::insertOperator(const Operator& arg) {
  if (!insertionPoint_->filled()) {
    if (arg.canBeUnary()) {
      TreeNode* newOperation = new UnaryNode(arg);
      insertionPoint_->addChild(newOperation);
      insertionPoint_ = newOperation;
    }
    else
      throw Exceptions::UnexpectedOperator();
  }
  else {
    while (!rootReached() && 
	   insertionPoint_->getPriority() >= arg.binaryPriority())
      ascend();
      
    TreeNode* oldChild = insertionPoint_->popChild();
    TreeNode* newChild = new BinaryNode(arg);
    newChild->addChild(oldChild);
    insertionPoint_->addChild(newChild);

    insertionPoint_ = newChild;
  }
}

void EvaluationTree::insertSubTree(const EvaluationTree& subtree) {
  if (insertionPoint_->filled())
    throw Exceptions::UnexpectedOperand();

  insertionPoint_->addChild(subtree.getRoot()->popChild());
}
