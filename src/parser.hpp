#pragma once

#include <vector>
#include <iostream>
#include <variant>
#include <optional>
#include "./arenaAllocator.hpp"

struct NodeTermIntLit
{
  Token int_lit;
};

struct NodeTermIdent
{
  Token ident;
};

struct NodeTerm
{
  std::variant<NodeTermIntLit *, NodeTermIdent *> val;
};

struct NodeExpr;

struct NodeBinExprAdd
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExprMul
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExpr
{
  std::variant <NodeBinExprAdd *,NodeBinExprMul *> op;

};

struct NodeExpr
{
  std::variant<NodeTerm *, NodeBinExpr *> var;
};

struct NodeStmtExit
{
  NodeExpr *expr;
};

struct NodeStmtConst
{
  Token ident;
  NodeExpr *expr;
};

struct NodeStmt
{
  std::variant<NodeStmtExit *, NodeStmtConst *> stmt;
};

struct NodeProg
{
  std::vector<NodeStmt *> stmts;
};

class Parser
{
public:
  explicit Parser(std::vector<Token> token_vec)
      : tokens(std::move(token_vec)), allocator(1024 * 1024 * 4) {}

  std::optional<NodeTerm *> parse_term()
  {
    if (auto int_lit_token = try_consume(TokenType::int_lit))
    {
      auto *node_term = allocator.alloc<NodeTerm>();
      auto *node_int_lit = allocator.alloc<NodeTermIntLit>();
      node_int_lit->int_lit = int_lit_token.value();
      node_term->val = node_int_lit;
      return node_term;
    }
    else if (auto ident_token = try_consume(TokenType::ident))
    {
      auto *node_term = allocator.alloc<NodeTerm>();
      auto *node_ident = allocator.alloc<NodeTermIdent>();
      node_ident->ident = ident_token.value();
      node_term->val = node_ident;
      return node_term;
    }
    return std::nullopt;
  }

  std::optional<NodeExpr *> parse_expr()
  {
    if (auto term = parse_term())
    {
      if (try_consume(TokenType::plus))
      {
        auto bin_expr = allocator.alloc<NodeBinExpr>();
        auto bin_expr_add = allocator.alloc<NodeBinExprAdd>();
        auto bin_expr_add_lhs = allocator.alloc<NodeExpr>();
        bin_expr_add_lhs->var = term.value();
        if (auto rhs = parse_expr())
        {
          bin_expr_add->rhs = rhs.value();
          bin_expr_add->lhs = bin_expr_add_lhs;
          bin_expr->op = bin_expr_add;
          auto expr = allocator.alloc<NodeExpr>();
          expr->var = bin_expr;
          return expr;
        }
        else
        {
          std::cerr << "Expected expression\n";
          std::exit(EXIT_FAILURE);
        }
      }
      else if(try_consume(TokenType::mul))
       {
        auto bin_expr = allocator.alloc<NodeBinExpr>();
        auto bin_expr_mul = allocator.alloc<NodeBinExprMul>();
        auto bin_expr_mul_lhs = allocator.alloc<NodeExpr>();
        bin_expr_mul_lhs->var = term.value();
        if (auto rhs = parse_expr())
        {
          bin_expr_mul->rhs = rhs.value();
          bin_expr_mul->lhs = bin_expr_mul_lhs;
          bin_expr->op = bin_expr_mul;
          auto expr = allocator.alloc<NodeExpr>();
          expr->var = bin_expr;
          return expr;
        }
        else
        {
          std::cerr << "Expected expression\n";
          std::exit(EXIT_FAILURE);
        }
      }
      else
      {
        auto *node_expr = allocator.alloc<NodeExpr>();
        node_expr->var = term.value();
        return node_expr;
      }
    }
    return std::nullopt;
  }

  std::optional<NodeStmt *> parse_stmt()
  {
    if (peek().has_value() && peek()->type == TokenType::exit)
    {
      consume();
      auto *node_stmt_exit = allocator.alloc<NodeStmtExit>();
      auto *node_stmt = allocator.alloc<NodeStmt>();

      if (auto node_expr = parse_expr())
      {
        node_stmt_exit->expr = node_expr.value();
        node_stmt->stmt = node_stmt_exit;
        if (!try_consume(TokenType::semi))
        {
          std::cerr << "Expected semi\n";
          std::exit(EXIT_FAILURE);
        }
        return node_stmt;
      }
      else
      {
        std::cerr << "Expected Expression\n";
        std::exit(EXIT_FAILURE);
      }
    }
    else if (peek().has_value() && peek()->type == TokenType::cnst && peek(1).has_value() && peek(1).value().type == TokenType::ident && peek(2).has_value() && peek(2).value().type == TokenType::eq)
    {
      consume();
      auto *node_stmt_const = allocator.alloc<NodeStmtConst>();
      auto *node_stmt = allocator.alloc<NodeStmt>();
      node_stmt_const->ident = consume();
      consume();
      if (auto node_expr = parse_expr())
      {
        node_stmt_const->expr = node_expr.value();
        if (!try_consume(TokenType::semi))
        {
          std::cerr << "Expected semi\n";
          std::exit(EXIT_FAILURE);
        }
      }
      else
      {
        std::cerr << "Expected Expression\n";
        std::exit(EXIT_FAILURE);
      }

      node_stmt->stmt = node_stmt_const;
      return node_stmt;
    }
    return std::nullopt;
  }

  std::optional<NodeProg> parse_prog()
  {
    NodeProg prog;

    while (peek().has_value())
    {

      if (auto node_stmt = parse_stmt())
      {
        prog.stmts.push_back(node_stmt.value());
      }
      else
      {
        std::cerr << "Expected statement\n";
        std::exit(EXIT_FAILURE);
      }
    }

    return prog;
  }

  NodeProg parse()
  {
    if (auto prog = parse_prog())
    {
      return prog.value();
    }
    else
    {
      std::cerr << "Expected Program\n";
      std::exit(EXIT_FAILURE);
    }
  }

private:
  std::optional<Token> peek(int offset = 0)
  {
    if (index + offset >= tokens.size())
    {
      return std::nullopt;
    }
    return tokens[index + offset];
  }

  Token consume()
  {
    return tokens[index++];
  }

  std::optional<Token> try_consume(TokenType type)
  {
    if (auto t = peek(); t && t->type == type)
    {
      return consume();
    }
    return std::nullopt;
  }

  std::vector<Token> tokens;
  size_t index = 0;
  ArenaAllocator allocator;
};
