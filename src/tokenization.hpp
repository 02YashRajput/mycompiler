#pragma once
#include <optional>
#include <string>
#include <stdexcept>
#include <unordered_map>

enum class TokenType
{
  exit,
  int_lit,
  semi,
  ident,
  cnst,
  assign,
  plus,
  mul,
  sub,
  div,
  open_paren,
  close_paren,
  open_curly,
  close_curly,
  eq,
  lt,
  gt,
  lte,
  gte,
  neq,
  if_,

};

struct Token
{
  TokenType type;
  std::optional<std::string> val;
  Token(TokenType t, std::optional<std::string> v = std::nullopt)
      : type(t), val(std::move(v)) {}
};

class Tokeniser
{
public:
  explicit Tokeniser(const std::string &contents) : src(std::move(contents))
  {
  }

  std::vector<Token> tokenise()
  {
    std::vector<Token> tokens;
    std::string buf = "";
    // Map for single-character tokens
    const std::unordered_map<char, TokenType> singleCharTokens = {
        {';', TokenType::semi},
        {'=', TokenType::assign},
        {'+', TokenType::plus},
        {'*', TokenType::mul},
        {'-', TokenType::sub},
        {'/', TokenType::div},
        {'(', TokenType::open_paren},
        {')', TokenType::close_paren},
        {'{', TokenType::open_curly},
        {'}', TokenType::close_curly}};

    const std::unordered_map<std::string, TokenType> doubleCharTokens = {
        {"==", TokenType::eq},
        {"!=", TokenType::neq},
        {"<=", TokenType::lte},
        {">=", TokenType::gte}};

    // Map for keywords
    const std::unordered_map<std::string, TokenType> keywords = {
        {"exit", TokenType::exit},
        {"const", TokenType::cnst},

    };

    while (peek().has_value())
    {
      char c = peek().value();

      if (std::isalpha(c))
      {
        buf.push_back(consume());
        while (peek().has_value() && std::isalnum(peek().value()))
        {
          buf.push_back(consume());
        }

        auto it = keywords.find(buf);
        if (it != keywords.end())
        {
          tokens.emplace_back(it->second);
        }
        else
        {
          tokens.emplace_back(TokenType::ident, buf);
        }
        buf.clear();
      }
      else if (std::isdigit(c))
      {
        buf.push_back(consume());
        while (peek().has_value() && std::isdigit(peek().value()))
        {
          buf.push_back(consume());
        }
        tokens.emplace_back(TokenType::int_lit, buf);
        buf.clear();
      }
      else if (std::isspace(c))
      {
        if (c == '\n' && (tokens[tokens.size() - 1].type != TokenType::semi && tokens[tokens.size() - 1].type != TokenType::open_curly && tokens[tokens.size() - 1].type != TokenType::close_curly))
        {
          std::cerr << "Wrong input: expected semi " << c << "\n";
          std::exit(EXIT_FAILURE);
        }
        consume();
      }
      else
      {
        // Check for two-character operators first
        if (peek(1).has_value())
        {
          std::string twoCharOp = {peek().value(), peek(1).value()};
          auto it2 = doubleCharTokens.find(twoCharOp);
          if (it2 != doubleCharTokens.end())
          {
            tokens.emplace_back(it2->second);
            consume();
            consume();
            continue;
          }
        }

        // Then fallback to single-char tokens
        auto it = singleCharTokens.find(c);
        if (it != singleCharTokens.end())
        {
          tokens.emplace_back(it->second);
          consume();
        }
        else
        {
          std::cerr << "Wrong input: unknown character '" << c << "'\n";
          std::exit(EXIT_FAILURE);
        }
      }
    }
  }

  return tokens;
} std::optional<char> peek(int offset = 0)
{
  if (index + offset >= src.size())
  {
    return {};
  }
  else
  {
    return src[index + offset];
  }
}

char consume()
{
  return src[index++];
}

const std::string src;
size_t index = 0;
}
;
