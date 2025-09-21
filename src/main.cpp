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
    case TokenType::print:
        return "print";
    case TokenType::if_:
        return "if";
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
    case TokenType::eq:
        return "==";
    case TokenType::neq:
        return "!=";
    case TokenType::mod:
        return "%";
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
void printNodeStmtScope(NodeStmtScope *node, int indent);
void printNodeProg(NodeProg &prog, int indent = 0);

#include <iostream>
#include <variant>
#include <vector>
#include <optional>

// --- Helper functions ---
void printIndent(int level)
{
    for (int i = 0; i < level; i++)
        std::cout << "  ";
}

// --- Terminals ---
void printNodeTermIntLit(NodeTermIntLit *node, int indent)
{
    printIndent(indent);
    std::cout << "IntLit(" << (node ? node->int_lit.val.value_or("") : "") << ")\n";
}

void printNodeTermIdent(NodeTermIdent *node, int indent)
{
    printIndent(indent);
    std::cout << "Ident(" << (node ? node->ident.val.value_or("") : "") << ")\n";
}

void printNodeTermParen(NodeTermParen *node, int indent)
{
    printIndent(indent);
    std::cout << "Paren:\n";
    if (node && node->expr)
        printNodeExpr(node->expr, indent + 1);
}

// --- Term ---
void printNodeTerm(NodeTerm *node, int indent)
{
    printIndent(indent);
    std::cout << "Term:\n";

    if (!node)
        return;

    if (std::holds_alternative<NodeTermIntLit *>(node->val))
        printNodeTermIntLit(std::get<NodeTermIntLit *>(node->val), indent + 1);
    else if (std::holds_alternative<NodeTermIdent *>(node->val))
        printNodeTermIdent(std::get<NodeTermIdent *>(node->val), indent + 1);
    else if (std::holds_alternative<NodeTermParen *>(node->val))
        printNodeTermParen(std::get<NodeTermParen *>(node->val), indent + 1);
}

// --- Binary expressions ---
void printNodeBinExpr(NodeBinExpr *node, int indent)
{
    printIndent(indent);
    std::cout << "BinExpr:\n";

    if (!node)
        return;

    std::visit([&](auto *opNode)
               {
                   if (!opNode)
                       return;
                   using T = std::decay_t<decltype(opNode)>;

                   // Print operator label at indent + 1
                   printIndent(indent + 1);
                   if constexpr (std::is_same_v<T, NodeBinExprAdd *>)
                       std::cout << "AddExpr:\n";
                   else if constexpr (std::is_same_v<T, NodeBinExprSub *>)
                       std::cout << "SubExpr:\n";
                   else if constexpr (std::is_same_v<T, NodeBinExprMul *>)
                       std::cout << "MulExpr:\n";
                   else if constexpr (std::is_same_v<T, NodeBinExprDiv *>)
                       std::cout << "DivExpr:\n";
                   else if constexpr (std::is_same_v<T, NodeBinExprMod *>)
                       std::cout << "ModExpr:\n";
                   else if constexpr (std::is_same_v<T, NodeBinExprEq *>)
                       std::cout << "EqExpr:\n";
                   else if constexpr (std::is_same_v<T, NodeBinExprNeq *>)
                       std::cout << "NeqExpr:\n";
                   else if constexpr (std::is_same_v<T, NodeBinExprLt *>)
                       std::cout << "LtExpr:\n";
                   else if constexpr (std::is_same_v<T, NodeBinExprGt *>)
                       std::cout << "GtExpr:\n";
                   else if constexpr (std::is_same_v<T, NodeBinExprLte *>)
                       std::cout << "LteExpr:\n";
                   else if constexpr (std::is_same_v<T, NodeBinExprGte *>)
                       std::cout << "GteExpr:\n";

                   // Print left and right expressions at indent + 2
                   if (opNode->lhs)
                       printNodeExpr(opNode->lhs, indent + 2);
                   if (opNode->rhs)
                       printNodeExpr(opNode->rhs, indent + 2); },
               node->op);
}

// --- Expressions ---
void printNodeExpr(NodeExpr *node, int indent)
{
    printIndent(indent);
    std::cout << "Expr:\n";
    if (!node)
        return;

    if (std::holds_alternative<NodeTerm *>(node->var))
    {
        auto t = std::get<NodeTerm *>(node->var);
        if (t)
            printNodeTerm(t, indent + 1);
    }
    else if (std::holds_alternative<NodeBinExpr *>(node->var))
    {
        auto b = std::get<NodeBinExpr *>(node->var);
        if (b)
            printNodeBinExpr(b, indent + 1);
    }
}

// --- Statements ---
void printNodeStmtExit(NodeStmtExit *node, int indent)
{
    printIndent(indent);
    std::cout << "ExitStmt:\n";
    if (node && node->expr)
        printNodeExpr(node->expr, indent + 1);
}

void printNodeStmtConst(NodeStmtConst *node, int indent)
{
    printIndent(indent);
    std::cout << "ConstDecl:\n";

    if (node)
    {
        printIndent(indent + 1);
        std::cout << "Ident(" << node->ident.val.value_or("") << ")\n";
        if (node->expr)
            printNodeExpr(node->expr, indent + 1);
    }
}

void printNodeStmtIf(NodeStmtIf *node, int indent)
{
    printIndent(indent);
    std::cout << "IfStmt:\n";

    if (!node)
        return;

    printIndent(indent + 1);
    std::cout << "Condition:\n";
    if (node->expr)
        printNodeExpr(node->expr, indent + 2);

    printIndent(indent + 1);
    std::cout << "Then:\n";
    if (node->scope)
        printNodeStmtScope(node->scope, indent + 2);
}

void printNodeStmtScope(NodeStmtScope *node, int indent)
{
    printIndent(indent);
    std::cout << "Scope {\n";

    if (!node || node->stmts.empty())
    {
        printIndent(indent + 1);
        std::cout << "(empty)\n";
    }
    else
    {
        for (auto *stmt : node->stmts)
            if (stmt)
                printNodeStmt(stmt, indent + 1);
    }

    printIndent(indent);
    std::cout << "}\n";
}

void printNodeStmtPrint(NodeStmtPrint *node, int indent)
{
    printIndent(indent);
    std::cout << "PrintStmt:\n";
    if (node && node->expr)
        printNodeExpr(node->expr, indent + 1);
}

void printNodeStmt(NodeStmt *node, int indent)
{
    printIndent(indent);
    std::cout << "Stmt:\n";

    if (!node)
        return;

    if (std::holds_alternative<NodeStmtExit *>(node->stmt))
        printNodeStmtExit(std::get<NodeStmtExit *>(node->stmt), indent + 1);
    else if (std::holds_alternative<NodeStmtConst *>(node->stmt))
        printNodeStmtConst(std::get<NodeStmtConst *>(node->stmt), indent + 1);
    else if (std::holds_alternative<NodeStmtScope *>(node->stmt))
        printNodeStmtScope(std::get<NodeStmtScope *>(node->stmt), indent + 1);
    else if (std::holds_alternative<NodeStmtIf *>(node->stmt))
        printNodeStmtIf(std::get<NodeStmtIf *>(node->stmt), indent + 1);
    else if (std::holds_alternative<NodeStmtPrint *>(node->stmt))
        printNodeStmtPrint(std::get<NodeStmtPrint *>(node->stmt), indent + 1);
}

// --- Program ---
void printNodeProg(NodeProg &prog, int indent)
{
    printIndent(indent);
    std::cout << "Program:\n";

    for (auto *stmt : prog.stmts)
        if (stmt)
            printNodeStmt(stmt, indent + 1);
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
    system("nasm -felf64 print_int.asm -o print_int.o");
    system("nasm -felf64 out.asm -o out.o");
    system("ld -o out out.o print_int.o");
    std::cout << "Ans: ";
    return EXIT_SUCCESS;
}