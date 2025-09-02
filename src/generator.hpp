#include <vector>
#include <sstream>
#include <unordered_map>

class Generator
{

public:
  explicit Generator(NodeProg program) : prog(std::move(program)) {}

  void gen_term(const NodeTerm* term) {
    struct TermVisitor{
      Generator* gen;
      void operator()(const NodeTermIntLit* term_int_lit)const{
        gen -> output << "    mov rax, " << term_int_lit->int_lit.val.value() << "\n";
        gen -> push("rax");
      }
      void operator()(const NodeTermIdent* term_ident)const{
          if(!gen->vars.contains(term_ident->ident.val.value())){
            std::cerr << "Variable " << term_ident->ident.val.value() << " not declared" << std::endl;
            exit(EXIT_FAILURE);
          }
          const auto& var = gen->vars.at(term_ident->ident.val.value());
          std::stringstream offset;
          offset << "QWORD [rsp + " << (gen->stack_size - var.stack_loc - 1) * 8 << "]\n";
          gen->push(offset.str());
    
      }
    };
    TermVisitor visitor(this);
    std::visit(visitor, term -> val);

  }



  void gen_expr(const NodeExpr* expr)
  {
    struct ExprVisistor{
      Generator* gen;
      void operator()(const NodeTerm* term)const{
        gen -> gen_term(term);
      }
      void operator()(const NodeBinExpr* bin_expr)const{
        gen -> gen_expr(bin_expr->add->lhs);
        gen -> gen_expr(bin_expr->add->rhs);
        gen -> pop("rax");
        gen -> pop("rbx");
        gen -> output << "    add rax, rbx\n";
        gen -> push("rax");

      }
    };
    ExprVisistor visitor(this);
    std::visit(visitor,expr->var);
  }

  void gen_stmt(const NodeStmt* stmt)
  {
    struct StmtVisitor{
        Generator* gen;
        void operator()(const NodeStmtExit* stmt_exit)const{
          gen -> gen_expr(stmt_exit -> expr);
          gen -> output << "    mov rax, 60\n";
          gen -> pop("rdi");
          gen -> output << "    syscall\n";
        }
        void operator()(const NodeStmtConst* stmt_const)const{
          if(gen->vars.contains(stmt_const->ident.val.value())){
            std::cerr << "Variable " << stmt_const->ident.val.value() << " already declared" << std::endl;
            exit(EXIT_FAILURE);
          }
          gen -> vars.insert({stmt_const -> ident.val.value(), Var(gen->stack_size)});
          gen -> gen_expr(stmt_const -> expr);
        }
    };
    StmtVisitor visitor{this};
    std::visit(visitor, stmt->stmt);
  }

  std::string gen_prog()
  {

    output << "global _start\n_start:\n";

    for (const NodeStmt *stmt : prog.stmts)
    {
      gen_stmt(stmt);
    }

    output << "    mov rax, 60\n";
    output << "    mov rdi, 0\n";
    output << "    syscall\n";

    return output.str();
  }


private:

  void push(const std::string& reg){
    output << "    push " << reg << "\n";
    stack_size++;
  }
  void pop(const std::string& reg){
    output << "    pop " << reg << "\n";
    stack_size--;
  }
  struct Var{
    size_t stack_loc;
  };



  std::stringstream output;
  const NodeProg prog;
  size_t stack_size = 0;
  std::unordered_map<std::string, Var> vars {};
};