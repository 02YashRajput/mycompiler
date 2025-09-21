#include <vector>
#include <sstream>
#include <unordered_map>

class Generator
{

public:
  explicit Generator(NodeProg program) : prog(std::move(program)) {}

  void gen_term(const NodeTerm *term)
  {
    struct TermVisitor
    {
      Generator *gen;
      void operator()(const NodeTermIntLit *term_int_lit) const
      {
        gen->output << "    mov rax, " << term_int_lit->int_lit.val.value() << "\n";
        gen->push("rax");
      }
      void operator()(const NodeTermIdent *term_ident) const
      {
        if (!gen->globals.contains(term_ident->ident.val.value()))
        {
          std::cerr << "Variable " << term_ident->ident.val.value() << " not declared" << std::endl;
          exit(EXIT_FAILURE);
        }
        const auto &var = gen->globals.at(term_ident->ident.val.value());
        std::stringstream offset;
        offset << "QWORD [rsp + " << (gen->stack_size - var.stack_loc - 1) * 8 << "]\n";
        gen->push(offset.str());
      }
      void operator()(const NodeTermParen *term_paren) const
      {
        gen->gen_expr(term_paren->expr);
      }
    };
    TermVisitor visitor(this);
    std::visit(visitor, term->val);
  }

  void gen_bin_expr(const NodeBinExpr *bin_expr)
  {
    struct BinExprVisitor
    {
      Generator *gen;
      void operator()(const NodeBinExprAdd *add) const
      {
        gen->gen_expr(add->lhs);
        gen->gen_expr(add->rhs);
        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    add rax, rbx\n";
        gen->push("rax");
      }
      void operator()(const NodeBinExprMul *mul) const
      {
        gen->gen_expr(mul->lhs);
        gen->gen_expr(mul->rhs);
        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    mul rbx\n";
        gen->push("rax");
      }

      void operator()(const NodeBinExprSub *sub) const
      {
        gen->gen_expr(sub->rhs);
        gen->gen_expr(sub->lhs);
        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    sub rax, rbx\n";
        gen->push("rax");
      }

      void operator()(const NodeBinExprDiv *div) const
      {
        gen->gen_expr(div->rhs);
        gen->gen_expr(div->lhs);
        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    cqo\n";      // sign-extend RAX -> RDX:RAX
        gen->output << "    idiv rbx\n"; // RAX/RBX -> quotient in RAX, remainder in RDX
        gen->push("rdx");
      }

      void operator()(const NodeBinExprMod *mod) const
      {
        gen->gen_expr(mod->rhs);
        gen->gen_expr(mod->lhs);
        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    cqo\n";      // sign-extend RAX -> RDX:RAX
        gen->output << "    idiv rbx\n"; // RAX/RBX -> quotient in RAX, remainder in RDX
        gen->push("rdx");
      }

      void operator()(const NodeBinExprEq *eq) const
      {
        gen->gen_expr(eq->rhs);
        gen->gen_expr(eq->lhs);
        gen->pop("rax");
        gen->pop("rbx");
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    sete al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
      }

      void operator()(const NodeBinExprNeq *neq) const
      {
        gen->gen_expr(neq->rhs);
        gen->gen_expr(neq->lhs);
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    setne al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
      }

      void operator()(const NodeBinExprLt *lt) const
      {
        gen->gen_expr(lt->rhs);
        gen->gen_expr(lt->lhs);
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    setl al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
      }

      void operator()(const NodeBinExprGt *gt) const
      {
        gen->gen_expr(gt->rhs);
        gen->gen_expr(gt->lhs);
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    setg al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
      }

      void operator()(const NodeBinExprLte *lte) const
      {
        gen->gen_expr(lte->rhs);
        gen->gen_expr(lte->lhs);
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    setle al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
      }

      void operator()(const NodeBinExprGte *gte) const
      {
        gen->gen_expr(gte->rhs);
        gen->gen_expr(gte->lhs);
        gen->pop("rax"); // lhs
        gen->pop("rbx"); // rhs
        gen->output << "    cmp rax, rbx\n";
        gen->output << "    setge al\n";
        gen->output << "    movzx rax, al\n";
        gen->push("rax");
      }
    };
    BinExprVisitor visitor(this);
    std::visit(visitor, bin_expr->op);
  }

  void gen_expr(const NodeExpr *expr)
  {
    struct ExprVisistor
    {
      Generator *gen;
      void operator()(const NodeTerm *term) const
      {
        gen->gen_term(term);
      }
      void operator()(const NodeBinExpr *bin_expr) const
      {
        gen->gen_bin_expr(bin_expr);
      }
    };
    ExprVisistor visitor(this);
    std::visit(visitor, expr->var);
  }

  void gen_scope(const NodeStmtScope *stmt_scope)
  {
    enter_scope();

    for (const NodeStmt *stmt_s : stmt_scope->stmts)
    {
      gen_stmt(stmt_s);
    }
    exit_scope();
  }

  void gen_exit()
  {
    output << "    mov rax, 1\n";
    output << "    mov rdi, 1\n";
    output << "    lea rsi, [rsp-1]\n";
    output << "    mov byte [rsp-1], 10\n";
    output << "    mov rdx, 1\n";
    output << "    syscall\n";
    output << "    mov rax, 60\n";
    pop("rdi");
    output << "    syscall\n";
    is_terminated = true;
  }

  void gen_stmt(const NodeStmt *stmt)
  {
    struct StmtVisitor
    {
      Generator *gen;
      void operator()(const NodeStmtExit *stmt_exit) const
      {
        gen->gen_expr(stmt_exit->expr);
        gen->gen_exit();
      }
      void operator()(const NodeStmtPrint *stmt_print) const
      {
        gen->gen_expr(stmt_print->expr);
        gen->pop("rdi");
        gen->output << "    call print_int\n";
      }
      void operator()(const NodeStmtIf *stmt_if) const
      {
        gen->gen_expr(stmt_if->expr);
        gen->pop("rax");
        std::string label = gen->create_label();
        gen->output << "    test rax, rax\n";
        gen->output << "    jz " << label << "\n";
        gen->gen_scope(stmt_if->scope);
        gen->output << label << ":\n";
      }
      void operator()(const NodeStmtConst *stmt_const) const
      {
        if (gen->is_declared(stmt_const->ident.val.value()))
        {
          std::cerr << "Variable " << stmt_const->ident.val.value() << " already declared" << std::endl;
          exit(EXIT_FAILURE);
        }
        gen->declare_var(stmt_const->ident.val.value(), Var(gen->stack_size));
        gen->gen_expr(stmt_const->expr);
      }
      void operator()(const NodeStmtScope *stmt_scope) const
      {
        gen->gen_scope(stmt_scope);
      }
    };
    StmtVisitor visitor{this};
    std::visit(visitor, stmt->stmt);
  }

  std::string gen_prog()
  {

    output << "extern print_int\nglobal _start\n_start:\n";

    for (const NodeStmt *stmt : prog.stmts)
    {
      if (is_terminated)
        return output.str();
      gen_stmt(stmt);
    }

    gen_exit();

    return output.str();
  }

private:
  struct Var
  {
    size_t stack_loc;
  };

  struct ScopeEntry
  {
    std::string name;
    std::optional<Var> old_binding; // empty if no shadowing
  };

  void push(const std::string &reg)
  {
    output << "    push " << reg << "\n";
    stack_size++;
  }
  void pop(const std::string &reg)
  {
    output << "    pop " << reg << "\n";
    stack_size--;
  }
  std::string create_label()
  {
    std::stringstream ss;
    ss << "label" << label_count++;
    return ss.str();
  }

  void
  enter_scope()
  {
    scopes.push_back({});
  }

  void exit_scope()
  {
    for (auto &entry : scopes.back())
    {
      if (entry.old_binding.has_value())
      {
        // Restore shadowed variable
        globals[entry.name] = entry.old_binding.value();
      }
      else
      {
        // Variable was new, remove it
        globals.erase(entry.name);
      }
    }
    scopes.pop_back();
  }

  void declare_var(const std::string &name, Var var)
  {
    std::optional<Var> old_binding;
    if (globals.contains(name))
    {
      old_binding = globals[name];
    }

    globals[name] = var;

    if (scopes.empty())
    {
      scopes.push_back({});
    }
    scopes.back().push_back({name, old_binding});
  }

  bool is_declared(const std::string &name) const
  {
    if (scopes.empty())
      return false;
    for (auto &entry : scopes.back())
    {
      if (entry.name == name)
      {
        return true; // declared in this scope
      }
    }
    return false;
  }

  bool is_terminated = false;
  std::stringstream output;
  const NodeProg prog;
  size_t stack_size = 0;
  int label_count = 0;
  std::unordered_map<std::string, Var> globals{};
  std::vector<std::vector<ScopeEntry>> scopes;
};