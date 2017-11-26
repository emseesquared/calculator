#ifndef __TREE_H__
#define __TREE_H__

#include <iostream>

using OperandType = double;

class TreeNode {
public:
  virtual ~TreeNode() = default;

  TreeNode* getParent() const { 
    return parent_;
  }

  void setParent(TreeNode* newParent) {
    parent_ = newParent;
  }

  virtual short getPriority() const = 0;

  virtual void addChild(TreeNode*) = 0;
  virtual TreeNode* popChild() = 0;

  virtual bool filled() const = 0;
  virtual double evaluate() const = 0;

protected:
  void addChildRoutine(TreeNode** ptrToChild, TreeNode*);
  TreeNode* popChildRoutine(TreeNode** ptrToChild);

private:
  TreeNode* parent_;
};

class Operator {
public:
  Operator(const char c): type_(c) { }

  short unaryPriority() const;
  short binaryPriority() const;

  bool canBeUnary() const;

  double operator()(const double) const;
  double operator()(const double, const double) const;

private:
  char type_;
};

class RootNode: public TreeNode {
public:
  ~RootNode() override;

  short getPriority() const override;

  void addChild(TreeNode* node) override;
  TreeNode* popChild() override;

  bool filled() const override {
    return firstChild_;
  }
  double evaluate() const override;

private:
  TreeNode* firstChild_ = nullptr;
};

class UnaryNode: public TreeNode {
public:
  UnaryNode(const Operator& arg): operator_(arg) { }
  ~UnaryNode() override;

  short getPriority() const override {
    return operator_.unaryPriority();
  }

  void addChild(TreeNode* node) override;
  TreeNode* popChild() override;

  bool filled() const override {
    return child_;
  }
  double evaluate() const override;

private:
  Operator operator_;
  TreeNode* child_ = nullptr;
};

class BinaryNode: public TreeNode {
public:
  BinaryNode(const Operator& arg): operator_(arg) { }

  ~BinaryNode() override;

  short getPriority() const override {
    return operator_.binaryPriority();
  }

  void addChild(TreeNode* node) override;
  TreeNode* popChild() override;

  bool filled() const override { 
    return leftChild_ && rightChild_; 
  }
  double evaluate() const override;

private:
  Operator operator_;

  TreeNode* leftChild_ = nullptr;
  TreeNode* rightChild_ = nullptr;
};

class Leaf: public TreeNode {
public:
  Leaf(const OperandType& operand): content_(operand) { }

  short getPriority() const override;

  void addChild(TreeNode*) override;
  TreeNode* popChild() override;

  bool filled() const override {
    return true; 
  }
  double evaluate() const override { 
    return content_;
  }

private:
  OperandType content_;
};

class EvaluationTree {
public:
  EvaluationTree();
  ~EvaluationTree();
  EvaluationTree(const EvaluationTree&) = delete;

  EvaluationTree(EvaluationTree&&);

  double evaluate() const {
    return root_->evaluate();
  }

  bool isReady() const;

  void insertOperand(const OperandType&);
  void insertOperator(const Operator&);
  void insertSubTree(const EvaluationTree&);

  TreeNode* getRoot() const {
    return root_;
  }

  bool rootReached() const {
    return insertionPoint_ == root_;
  }

  void ascend() {
    insertionPoint_ = insertionPoint_->getParent();
  }

private:
  TreeNode* root_;
  TreeNode* insertionPoint_;
};

#endif
