#include <cstdio>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "lexer.hh"
#include "parser.hh"

const std::string SOURCE_PATH = "Test/source.pas";
const std::string ERR_PATH = "Test/source.err";
const std::string DYD_PATH = "Test/source.dyd";
const std::string DYS_PATH = "Test/source.dys";
const std::string VAR_PATH = "Test/source.var";
const std::string PRO_PATH = "Test/source.pro";

int main() {
  std::cout.tie(nullptr), std::cerr.tie(nullptr);
  std::cout << "===========words===========" << std::endl;
  std::ifstream inputFile(SOURCE_PATH);
  std::vector<std::string> words;
  std::string line;
  while (std::getline(inputFile, line)) {
    std::istringstream iss(line);
    std::string word;
    while (iss >> word) {
      words.push_back(std::move(word));
    }
    words.push_back("\n");
  }

  std::freopen(ERR_PATH.c_str(), "w+", stderr);

  std::cout << "===========lexer===========" << std::endl;
  std::ofstream lexerFile(DYD_PATH);
  std::vector<Token> tokens;
  Lexer lexer(std::move(words));
  if (!lexer.good()) {
    std::cout << "Compiler aborted due to lexer error. A complete log of "
                 "this run can be found in: "
              << ERR_PATH << std::endl;
    return 1;
  }
  lexer.formatPrint(lexerFile);
  tokens = std::move(lexer.getTokens());

  std::cout << "===========parser===========" << std::endl;
  std::ofstream parserDysFile(DYS_PATH);
  std::ofstream parserVarFile(VAR_PATH);
  std::ofstream parserProFile(PRO_PATH);
  Parser parser(std::move(tokens));
  if (!parser.good()) {
    std::cout << "Compiler aborted due to parser error. A complete log of "
                 "this run can be found in: "
              << ERR_PATH << std::endl;
  }
  parser.formatPrint(parserDysFile, parserVarFile, parserProFile);

  return 0;
}