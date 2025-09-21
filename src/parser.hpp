#pragma once

#include <vector>
#include <iostream>
#include <variant>
#include <optional>
#include "./arenaAllocator.hpp"

struct NodeExpr;

struct NodeBinExprEq
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprNeq
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprLt
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprGt
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprLte
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprGte
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprMod
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprAdd
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprSub
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};
struct NodeBinExprDiv
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExprMul
{
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeTermIntLit
{
  Token int_lit;
};

struct NodeTermIdent
{
  Token ident;
};

struct NodeTermParen
{
  NodeExpr *expr;
};
struct NodeTerm
{
  std::variant<NodeTermIntLit *, NodeTermIdent *, NodeTermParen *> val;
};

struct NodeBinExpr
{
  std::variant<NodeBinExprAdd *, NodeBinExprMul *, NodeBinExprSub *, NodeBinExprDiv *, NodeBinExprMod *, NodeBinExprEq *, NodeBinExprGt *, NodeBinExprNeq *, NodeBinExprLte *, NodeBinExprGte *, NodeBinExprLt *> op;
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

struct NodeStmt;
struct NodeStmtScope
{
  std::vector<NodeStmt *> stmts;
};
struct NodeStmt
{
  std::variant<NodeStmtExit *, NodeStmtConst *, NodeStmtScope *> stmt;
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
    else if (auto open_paren = try_consume(TokenType::open_paren))
    {
      auto node_expr = parse_expr();
      if (node_expr.has_value())
      {
        if (auto close_paren = try_consume(TokenType::close_paren))
        {
          auto term_paren = allocator.alloc<NodeTermParen>();
          term_paren->expr = node_expr.value();
          auto node_term = allocator.alloc<NodeTerm>();
          node_term->val = term_paren;
          return node_term;
        }
        else
        {
          std::cerr << "Expected close parenthesis\n";
          std::exit(EXIT_FAILURE);
        }
      }
    }

    return std::nullopt;
  }

  std::optional<NodeExpr *> parse_expr(int min_prec = 0)
  {
    std::optional<NodeTerm *> term_lhs = parse_term();
    if (!term_lhs.has_value())
    {
      return std::nullopt;
    }

    auto expr_lhs = allocator.alloc<NodeExpr>();
    expr_lhs->var = term_lhs.value();

    while (true)
    {
      std::optional<Token> curr_tok = peek();
      int prec;
      if (curr_tok.has_value())
      {
        prec = get_precedence(curr_tok->type);
        if (prec < min_prec)
        {
          break;
        }
      }
      else
      {
        std::cerr << "Expected semi\n";
        std::exit(EXIT_FAILURE);
      }
      Token op = consume();
      int next_min_prec = prec + 1;
      auto expr_rhs = parse_expr(next_min_prec);
      if (!expr_rhs.has_value())
      {
        std::cerr << "Unable to parse expression" << std::endl;
        exit(EXIT_FAILURE);
      }
      auto bin_expr = allocator.alloc<NodeBinExpr>();
      if (op.type == TokenType::plus)
      {
        auto bin_expr_add = allocator.alloc<NodeBinExprAdd>();
        bin_expr_add->lhs = expr_lhs;
        bin_expr_add->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_add;
      }
      else if (op.type == TokenType::mul)
      {
        auto bin_expr_mul = allocator.alloc<NodeBinExprMul>();
        bin_expr_mul->lhs = expr_lhs;
        bin_expr_mul->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_mul;
      }
      else if (op.type == TokenType::sub)
      {
        auto bin_expr_sub = allocator.alloc<NodeBinExprSub>();
        bin_expr_sub->lhs = expr_lhs;
        bin_expr_sub->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_sub;
      }
      else if (op.type == TokenType::div)
      {
        auto bin_expr_div = allocator.alloc<NodeBinExprDiv>();
        bin_expr_div->lhs = expr_lhs;
        bin_expr_div->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_div;
      }
      else if (op.type == TokenType::mod)
      {
        auto bin_expr_mod = allocator.alloc<NodeBinExprMod>();
        bin_expr_mod->lhs = expr_lhs;
        bin_expr_mod->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_mod;
      }
      else if (op.type == TokenType::eq)
      {
        auto bin_expr_eq = allocator.alloc<NodeBinExprEq>();
        bin_expr_eq->lhs = expr_lhs;
        bin_expr_eq->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_eq;
      }
      else if (op.type == TokenType::neq)
      {
        auto bin_expr_neq = allocator.alloc<NodeBinExprNeq>();
        bin_expr_neq->lhs = expr_lhs;
        bin_expr_neq->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_neq;
      }
      else if (op.type == TokenType::lt)
      {
        auto bin_expr_lt = allocator.alloc<NodeBinExprLt>();
        bin_expr_lt->lhs = expr_lhs;
        bin_expr_lt->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_lt;
      }
      else if (op.type == TokenType::gt)
      {
        auto bin_expr_gt = allocator.alloc<NodeBinExprGt>();
        bin_expr_gt->lhs = expr_lhs;
        bin_expr_gt->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_gt;
      }
      else if (op.type == TokenType::lte)
      {
        auto bin_expr_lte = allocator.alloc<NodeBinExprLte>();
        bin_expr_lte->lhs = expr_lhs;
        bin_expr_lte->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_lte;
      }
      else if (op.type == TokenType::gte)
      {
        auto bin_expr_gte = allocator.alloc<NodeBinExprGte>();
        bin_expr_gte->lhs = expr_lhs;
        bin_expr_gte->rhs = expr_rhs.value();
        bin_expr->op = bin_expr_gte;
      }
      else
      {
        std::cerr << "Unexpected operation" << std::endl;
        exit(EXIT_FAILURE);
      }

      auto new_expr = allocator.alloc<NodeExpr>();
      new_expr->var = bin_expr;
      expr_lhs = new_expr;
    }
    return expr_lhs;
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
    else if (peek().has_value() && peek()->type == TokenType::cnst && peek(1).has_value() && peek(1).value().type == TokenType::ident && peek(2).has_value() && peek(2).value().type == TokenType::assign)
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
    else if (peek().has_value() && peek()->type == TokenType::open_curly)
    {
      consume();
      auto node_scope = allocator.alloc<NodeStmtScope>();
      while (peek().has_value() && peek().value().type != TokenType::close_curly)
      {
        if (auto stmt = parse_stmt())
        {
          node_scope->stmts.push_back(stmt.value());
        }
        else
        {
          std::cerr << "Expected statement inside scope\n";
          std::exit(EXIT_FAILURE);
        }
      }
      if (!try_consume(TokenType::close_curly))
      {
        std::cerr << "Expected '}'\n";
        std::exit(EXIT_FAILURE);
      }
      auto node_stmt = allocator.alloc<NodeStmt>();
      node_stmt->stmt = node_scope;
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

  int get_precedence(TokenType type)
  {
    auto it = precedence.find(type);
    if (it != precedence.end())
    {
      return it->second;
    }
    return -1;
  }

  std::unordered_map<TokenType, int> precedence = {
      {TokenType::eq, 0}, // ==, !=
      {TokenType::neq, 0},
      {TokenType::lt, 0}, // <, >, <=, >=
      {TokenType::gt, 0},
      {TokenType::lte, 0},
      {TokenType::gte, 0},

      {TokenType::plus, 1}, // +, -
      {TokenType::sub, 1},

      {TokenType::mul, 2}, // *, /, %
      {TokenType::div, 2},
  };

  std::vector<Token> tokens;
  size_t index = 0;
  ArenaAllocator allocator;
};
