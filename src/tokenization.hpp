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
  eq,
  plus,
  mul
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
        {'=', TokenType::eq},
        {'+', TokenType::plus},
        {'*', TokenType::mul}
    };

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
        if (c == '\n' && tokens[tokens.size() - 1].type != TokenType::semi)
        {
          std::cerr << "Wrong input: " << c << "\n";
          std::exit(EXIT_FAILURE);
        }
        consume();
      }
      else
      {
        auto it = singleCharTokens.find(c);
        if (it != singleCharTokens.end())
        {
          tokens.emplace_back(it->second);
          consume();
        }
        else
        {
          std::cerr << "Wrong input: " << c << "\n";
          std::exit(EXIT_FAILURE);
        }
      }
    }

    return tokens;
  }
  std::optional<char> peek(int offset = 0)
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
};
