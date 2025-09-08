#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "./tokenization.hpp"
#include "./parser.hpp"
#include "./generator.hpp"

std::string tokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::exit:
        return "exit";
    case TokenType::int_lit:
        return "int_lit";
    case TokenType::semi:
        return "semi";
    case TokenType::ident:
        return "ident";
    case TokenType::cnst:
        return "const";
    case TokenType::assign:
        return "=";
    case TokenType::plus:
        return "+";
    case TokenType::mul:
        return "*";
    case TokenType::sub:
        return "-";
    case TokenType::div:
        return "/";
    case TokenType::mod:
        return "%";
    case TokenType::eq:
        return "==";
    case TokenType::neq:
        return "!=";
    case TokenType::lt:
        return "<";
    case TokenType::gt:
        return ">";
    case TokenType::lte:
        return "<=";
    case TokenType::gte:
        return ">=";
    case TokenType::open_paren:
        return "(";
    case TokenType::close_paren:
        return ")";
    case TokenType::open_curly:
        return "{";
    case TokenType::close_curly:
        return "}";
    default:
        return "unknown";
    }
}


// Function to print tokens
void printTokens(const std::vector<Token> &tokens)
{
    for (const auto &token : tokens)
    {
        std::cout << "Token(" << tokenTypeToString(token.type);

        if (token.val.has_value())
        {
            std::cout << ", value=\"" << token.val.value() << "\"";
        }

        std::cout << ")\n";
    }
}
// --- Print function declarations ---
void printIndent(int level);
void printNodeTermIntLit(NodeTermIntLit *node, int indent = 0);
void printNodeTermIdent(NodeTermIdent *node, int indent = 0);
void printNodeTerm(NodeTerm *node, int indent = 0);
void printNodeExpr(NodeExpr *node, int indent = 0);
void printNodeBinExprAdd(NodeBinExprAdd *node, int indent = 0);
void printNodeBinExpr(NodeBinExpr *node, int indent = 0);
void printNodeStmtExit(NodeStmtExit *node, int indent = 0);
void printNodeStmtConst(NodeStmtConst *node, int indent = 0);
void printNodeStmt(NodeStmt *node, int indent = 0);
void printNodeStmtScope(NodeStmtScope* node, int indent);
void printNodeProg(NodeProg &prog, int indent = 0);

// --- Implementations ---

void printIndent(int level)
{
    for (int i = 0; i < level; i++)
    {
        std::cout << "  ";
    }
}

void printNodeTermParen(NodeTermParen *node, int indent)
{
    printIndent(indent);
    std::cout << "Paren:\n";
    printNodeExpr(node->expr, indent + 1);
}
void printNodeTermIntLit(NodeTermIntLit *node, int indent)
{
    printIndent(indent);
    std::cout << "IntLit(" << node->int_lit.val.value_or("") << ")\n";
}

void printNodeTermIdent(NodeTermIdent *node, int indent)
{
    printIndent(indent);
    std::cout << "Ident(" << node->ident.val.value_or("") << ")\n";
}

void printNodeTerm(NodeTerm *node, int indent)
{
    printIndent(indent);
    std::cout << "Term:\n";

    if (std::holds_alternative<NodeTermIntLit *>(node->val))
    {
        printNodeTermIntLit(std::get<NodeTermIntLit *>(node->val), indent + 1);
    }
    else if (std::holds_alternative<NodeTermIdent *>(node->val))
    {
        printNodeTermIdent(std::get<NodeTermIdent *>(node->val), indent + 1);
    }
    else if (std::holds_alternative<NodeTermParen *>(node->val))
    {
        printNodeTermParen(std::get<NodeTermParen *>(node->val), indent + 1);
    }
}

void printNodeBinExpr(NodeBinExpr *node, int indent)
{
    printIndent(indent);
    std::cout << "BinExpr:\n";

    std::visit([&](auto *opNode)
               {
        using T = std::decay_t<decltype(opNode)>;

        if constexpr (std::is_same_v<T, NodeBinExprAdd*>) {
            printIndent(indent + 1);
            std::cout << "AddExpr:\n";
            printNodeExpr(opNode->lhs, indent + 2);
            printNodeExpr(opNode->rhs, indent + 2);
        } 
        else if constexpr (std::is_same_v<T, NodeBinExprMul*>) {
            printIndent(indent + 1);
            std::cout << "MulExpr:\n";
            printNodeExpr(opNode->lhs, indent + 2);
            printNodeExpr(opNode->rhs, indent + 2);
        }
        else if constexpr (std::is_same_v<T, NodeBinExprSub*>) {
            printIndent(indent + 1);
            std::cout << "SubExpr:\n";
            printNodeExpr(opNode->lhs, indent + 2);
            printNodeExpr(opNode->rhs, indent + 2);
        }
        else if constexpr (std::is_same_v<T, NodeBinExprDiv*>) {
            printIndent(indent + 1);
            std::cout << "DivExpr:\n";
            printNodeExpr(opNode->lhs, indent + 2);
            printNodeExpr(opNode->rhs, indent + 2);
        } }, node->op);
}
void printNodeExpr(NodeExpr *node, int indent)
{
    printIndent(indent);
    std::cout << "Expr:\n";

    if (std::holds_alternative<NodeTerm *>(node->var))
    {
        printNodeTerm(std::get<NodeTerm *>(node->var), indent + 1);
    }
    else if (std::holds_alternative<NodeBinExpr *>(node->var))
    {
        printNodeBinExpr(std::get<NodeBinExpr *>(node->var), indent + 1);
    }
}

void printNodeStmtExit(NodeStmtExit *node, int indent)
{
    printIndent(indent);
    std::cout << "ExitStmt:\n";
    printNodeExpr(node->expr, indent + 1);
}

void printNodeStmtConst(NodeStmtConst *node, int indent)
{
    printIndent(indent);
    std::cout << "ConstDecl:\n";

    printIndent(indent + 1);
    std::cout << "Ident(" << node->ident.val.value_or("") << ")\n";

    printNodeExpr(node->expr, indent + 1);
}
void printNodeStmt(NodeStmt* node, int indent)
{
    printIndent(indent);
    std::cout << "Stmt:\n";

    if (std::holds_alternative<NodeStmtExit*>(node->stmt))
    {
        printNodeStmtExit(std::get<NodeStmtExit*>(node->stmt), indent + 1);
    }
    else if (std::holds_alternative<NodeStmtConst*>(node->stmt))
    {
        printNodeStmtConst(std::get<NodeStmtConst*>(node->stmt), indent + 1);
    }
    else if (std::holds_alternative<NodeStmtScope*>(node->stmt))
    {
        printNodeStmtScope(std::get<NodeStmtScope*>(node->stmt), indent + 1);
    }
}

void printNodeStmtScope(NodeStmtScope* node, int indent)
{
    printIndent(indent);
    std::cout << "Scope {\n";

    for (auto* stmt : node->stmts)
    {
        printNodeStmt(stmt, indent + 1);
    }

    printIndent(indent);
    std::cout << "}\n";
}

void printNodeProg(NodeProg &prog, int indent)
{
    printIndent(indent);
    std::cout << "Program:\n";

    for (auto *stmt : prog.stmts)
    {
        printNodeStmt(stmt, indent + 1);
    }
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        std::cout << "Wrong input format the input should be ./mycomiper <input file>";
        return EXIT_FAILURE;
    }

    std::string contents;

    {
        std::ifstream file(argv[1]);
        if (!file)
        {
            std::cerr << "Error: could not open file " << argv[1] << std::endl;
            return EXIT_FAILURE;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        contents = buffer.str();
    }

    // std::cout << "File contents:\n" << contents << std::endl;

    Tokeniser tokeniser(std::move(contents));
    std::vector<Token> tokens = tokeniser.tokenise();
    printTokens(tokens);

    Parser parser(std::move(tokens));

    NodeProg prog = parser.parse();
    printNodeProg(prog);

    Generator generator(std::move(prog));
    std::string output = generator.gen_prog();

    // std::cout<<output<<std::endl;

    {
        std::fstream file("out.asm", std::ios::out);
        file << output;
    }
    system("nasm -felf64 out.asm");
    system("ld -o out out.o");
    std::cout << "Ans: ";
    return EXIT_SUCCESS;
}